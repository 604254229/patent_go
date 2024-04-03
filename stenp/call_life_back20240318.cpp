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
    vector< double> vec_product;
    int col_size = cur_column_values[i].size();
    for (int j = 0; j < col_size; j++) {
      double value = cur_column_values[i][j];
      if (j == dao_index) {
        // 取倒数该行
        product *= 1.0 / value;
      } else {
        product = product * value;
      }
      if ((j == col_size - 1) || (product - 100000000000.0)>0) {
        // printf("\nproduct %f\n", product);
        product = std::pow(product, 1.0 / col_size);
        vec_product.push_back(product);
        product = 1.0;
      }
    }
     

    long double product_result = 1.0;
    for (const auto& item : vec_product) {
        printf("%lf, " ,item);
        product_result *= item;
    }
    std::cout << "row" << i << " product_result: " << product_result
              << std::endl;
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
      if (j == row_size - 1 || product > 100000000000.0)
        product = std::pow(product, 1.0 / row_size);
      vec_product.push_back(product);
      product = 1.0;
    }
    long double product_result = 1.0;
    for (const auto& item : vec_product) product_result *= item;
    result.push_back(product_result);
    // std::cout << "col" << j << " getNgeometricMean: " << product_result
    //           << std::endl;
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
  printf("max_diff %f, cur_diff %f, index %d\n",max_diff, cur_diff,dao_index );

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
    double sum = 0.0;
    for (int i = 0; i < div_column_values.size(); i++) {
      div_column_values[i][j] /= gc_n_result[j];
      sum += div_column_values[i][j];
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

int main() {
  // Open an Excel workbook
  XLDocument doc;
  doc.open("/home/wupengfei/stenp/build/20240307.xlsx");

  // Assume the first worksheet in the workbook
  auto worksheet = doc.workbook().worksheet("Sheet1");

  // Get the number of rows and columns
  auto row_count = worksheet.rowCount();
  auto col_count = worksheet.columnCount();
  printf("total row %d, col %d \n", row_count, col_count);

  // Initialize an array to store column values
  std::vector<std::vector<double>> column_values;
  // 获取当前测试数据索引下标7,3 -- 206,107
  for (unsigned int row = 7; row <= 206; ++row) {
    std::vector<double> tmp_row;
    for (unsigned int col = 3; col <= 107; ++col) {
      auto cell = worksheet.cell(row, col);
      // 血压数据拆分处理
      if (col == 4) {
        string value = cell.value().get<string>();
        int pos = value.find('/', 0);
        double d1 = atof(value.substr(0, pos).c_str());
        double d2 = atof(value.substr(pos + 1).c_str());
        tmp_row.push_back(d1);
        tmp_row.push_back(d2);

      } else {
        try {
          double value = cell.value().get<double>();
          tmp_row.push_back(value);
        } catch (const std::runtime_error&) {
          int64_t value = cell.value().get<int64_t>();
          tmp_row.push_back((double)value);
        }
      }
    }
    column_values.emplace_back(std::move(tmp_row));
  }

  outFile(column_values, "col_value.txt");

  // 步骤1.1 计算几何均值
  std::vector<double> gc_m_result;
  geometricMean(column_values, gc_m_result);

  // 步骤1.2 计算几个均值中最大值最小值

  double cur_max_diff = getMaxDiff(gc_m_result);
  exit(1);

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

  // 步骤3 对健康数据流求功能几何均值 对参数列取均值
  std::vector<double> gc_n_result;
  getNgeometricMean(cur_column_values, gc_n_result);
  // 步骤3.2 参数/该参数的gc_n 横向时间段的参数数据和
  std::vector<double> time_sum;
  std::vector<std::vector<double>> div_column_values(cur_column_values);
  getTimeSum(gc_n_result, div_column_values, time_sum);

  outFile(div_column_values, "div_value.txt");
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
