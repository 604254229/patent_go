#include <OpenXLSX.hpp>
#include <cmath>
#include <fstream>
#include <set>
#include <vector>
using namespace std;
using namespace OpenXLSX;

// #几何平均值 i 为是否需要取倒数的列
void geometricMean(const std::vector<std::vector<double>>& cur_column_values,
                   std::vector<double>& result, int dao_index = -1) {
  printf("row_size %zu, col_size%zu \n", cur_column_values.size(),
         cur_column_values[0].size());

  // Calculate geometric mean for each column
  for (int i = 0; i < cur_column_values.size(); i++) {
    double product = 1.0;
    vector<double> vec_product;
    int col_size = cur_column_values[i].size();
    for (int j = 0; j < col_size; j++) {
      double value = cur_column_values[i][j];
      if (j == dao_index) {
        // 取倒数该行
        product *= 1.0 / value;
      } else {
        product = product * value;
      }
      if ((j == col_size - 1) || (product - 100000000000.0) > 0) {
        product = std::pow(product, 1.0 / col_size);
        vec_product.push_back(product);
        product = 1.0;
      }
    }

    long double product_result = 1.0;
    for (const auto& item : vec_product) {
      printf("%lf, ", item);
      product_result *= item;
    }
    // std::cout << "row" << i << " product_result: " << product_result
    //           << std::endl;
    result.push_back(product_result);
  }
}

void getNgeometricMean(
    const std::vector<std::vector<double>>& cur_column_values,
    std::vector<double>& result, int dao_index = -1) {
  // Calculate geometric mean for each column

  for (int j = 0; j < cur_column_values[0].size(); j++) {
    double product = 1.0;
    int row_size = cur_column_values.size();
    vector<long double> vec_product;
    for (int i = 0; i < row_size; i++) {
      product *= cur_column_values[i][j];
      // 避免数据过大 提前求pow
      if (i == row_size - 1 || product > 100000000000.0) {
        product = std::pow(product, 1.0 / row_size);
        vec_product.push_back(product);
        product = 1.0;
      }
    }
    long double product_result = 1.0;
    for (const auto& item : vec_product) product_result *= item;
    result.push_back(product_result);
  }
}

double getMaxDiff(const vector<double>& result) {
  double min_num = result[0];
  double max_num = result[0];
  for (const auto& item : result) {
    min_num = std::min(item, min_num);
    max_num = std::max(item, max_num);
  }
  printf("max %f, min%f \n", max_num, min_num);
  return fabs(log(max_num) - log(min_num));
}

bool checkMaxAndsetDao(
    int dao_index, const std::vector<std::vector<double>>& cur_column_values,
    double& max_diff) {
  std::vector<double> cur_result;
  geometricMean(cur_column_values, cur_result, dao_index);
  double cur_diff = getMaxDiff(cur_result);
  printf("max_diff %f, cur_diff %f, index %d\n", max_diff, cur_diff, dao_index);

  if (cur_diff > max_diff) {
    // 更新 取倒数
    max_diff = cur_diff;
    return true;
  }

  return false;
}

void getTimeSum(const std::vector<double>& gc_n_result,
                std::vector<std::vector<double>>& div_column_values,
                std::vector<double>& time_sum) {
  for (int j = 0; j < div_column_values[0].size(); j++) {
    for (int i = 0; i < div_column_values.size(); i++) {
      div_column_values[i][j] /= gc_n_result[j];
    }
  }

  for (const auto& item : div_column_values) {
    double sum = 0.0;
    for (const auto& values : item) {
      sum += values;
    }
    time_sum.push_back(sum);
  }
}

void getSumPer(std::vector<std::vector<double>>& per_column_values,
               std::vector<double>& time_sum) {
  for (int i = 0; i < per_column_values.size(); i++) {
    for (int j = 0; j < per_column_values[i].size(); j++) {
      per_column_values[i][j] /= time_sum[i];
    }
  }
}

void outFile(const std::vector<std::vector<double>>& column_values,
             const string& file_name) {
  ofstream outfile;
  outfile.open(file_name);
  for (const auto& item : column_values) {
    for (const auto& values : item) outfile << values << ",";
    outfile << "\n";
  }
  outfile.close();
}

void outFileVec(const std::vector<double>& values, const string& file_name) {
  ofstream outfile;
  outfile.open(file_name);
  for (const auto& item : values) outfile << item << ",";
  outfile.close();
}

double calculateDiff(double a, double b) {
  return fabs(log(a / b) / 0.618);
}

int main() {
  vector<vector<double>> column_values{
      {7.88, 99.07, 79.49, 771.71, 221, 9.93, 12.39, 7.72, 5.87, 15.25, 1230},
      {7.5, 96.23, 75.34, 728.47, 213.4, 10.08, 11.96, 7.34, 4.88, 12.32,
       1374.13},
      {6.11, 107.61, 77.87, 668.91, 235.69, 7.63, 9.61, 5.96, 3.89, 11.35,
       1219.01}};

  outFile(column_values, "col_value.txt");

  // 步骤1.1 计算几何均值
  std::vector<double> gc_m_result;
  geometricMean(column_values, gc_m_result);

  outFileVec(gc_m_result, "gc_m_result.txt");

  // 步骤1.2 计算几个均值中最大值最小值

  double cur_max_diff = getMaxDiff(gc_m_result);

  // 步骤1.3 一一取倒数，第一轮取一个确定
  // 第二轮二个(保留前一轮的哪些倒数)。。。。最后算出一个差值最大的作为稳定数据流
  std::map<double, set<int>>
      map_gdmm2cols;  // 记录每次去倒数结果对应的是哪几列取了倒数
  std::vector<std::vector<double>> cur_column_values(column_values);

  int N = cur_column_values[0].size();

  map_gdmm2cols[cur_max_diff] = {};  // 先把没有任何取倒数结果加入

  std::set<int> set_daos;  // 记录前几轮已经取倒数了的列
  for (int i = 0; i < N; i++) {
    bool need_update = false;
    int need_updaate_index = -1;
    for (int j = 0; j < N; j++) {
      // 每次取一个
      if (set_daos.find(j) == set_daos.end()) {
        // 如果取倒数后计算的getMaxDiff更大 就更新倒数列 否则停止
        // 获取了健康数据流
        if (checkMaxAndsetDao(j, cur_column_values, cur_max_diff)) {
          // 该列需要更新
          need_update = true;
          need_updaate_index = j;
        }
      }
    }

    // 本轮有需要更新的则继续下一轮
    if (need_update) {
      set_daos.insert(need_updaate_index);
      for (int i = 0; i < cur_column_values.size(); i++)
        for (int j = 0; j < cur_column_values[i].size(); j++) {
          if (j == need_updaate_index) {
            cur_column_values[i][j] = 1.0 / cur_column_values[i][j];
            printf("need dao index %d \n", need_updaate_index);
          }
        }
    } else {
      // 跳出循环
      break;
    }
  }

  outFile(cur_column_values, "gcyy_value.txt");

  std::vector<double> gcyy_m_result;
  geometricMean(cur_column_values, gcyy_m_result);

  outFileVec(gcyy_m_result, "gcyy_m_result.txt");

  //计算相邻的占比
  std::vector<double> gcyy_diff_result;
  for (int i = 1; i < gcyy_m_result.size(); i++) {
    double diff = calculateDiff(gcyy_m_result[i], gcyy_m_result[i-1]);
    gcyy_diff_result.push_back(diff);
  }

    outFileVec(gcyy_diff_result, "gcyy_diff_result.txt");


  // 步骤3 对健康数据流求功能几何均值 对参数列取均值
  std::vector<double> gc_n_result;
  getNgeometricMean(cur_column_values, gc_n_result);

  outFileVec(gc_n_result, "gc_n_result.txt");



  // 步骤3.2 参数/该参数的gc_n 横向时间段的参数数据和
  std::vector<double> time_sum;
  std::vector<std::vector<double>> div_column_values(cur_column_values);
  getTimeSum(gc_n_result, div_column_values, time_sum);

  outFileVec(time_sum, "time_sum.txt");

  outFile(div_column_values, "div_value.txt");

    // success

  // 计算比例
  std::vector<std::vector<double>> per_column_values(div_column_values);
  getSumPer(per_column_values, time_sum);
  // 计算占比几何均值

  std::vector<double> per_gc_result;
  geometricMean(per_column_values, per_gc_result);

  outFile(per_column_values, "per_value.txt");

  // 步骤3.3

  return 0;
}
