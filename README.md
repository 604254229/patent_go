# gRPC Hello World

 1. 
    ```console
      protoc --go_out=. --go_opt=paths=source_relative \
    --go-grpc_out=. --go-grpc_opt=paths=source_relative \
    protobuf/patent.proto
    ```
 1. Run the server:

    ```console
    go run patent_server/main.go
    ```

 2. Run the client:

    ```console
    go run patent_client/main.go --name=Alice
    ```

