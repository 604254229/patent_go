package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"

	pb "github.com/604254229/patent_go/protobuf"
	"google.golang.org/grpc"
)

var (
	port = flag.Int("port", 50051, "The server port")
)

type server struct {
	pb.UnimplementedPatentSvrServer
}

func (s *server) GetPatentResult(ctx context.Context, in *pb.GetPatentResultRequest) (*pb.GetPatentResultReply, error) {
	log.Printf("Received: %v", in.GetName())
	return &pb.GetPatentResultReply{Message: "GetPatentResult " + in.GetName()}, nil
}

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterPatentSvrServer(s, &server{})
	log.Printf("server listening at %v", lis.Addr())
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
