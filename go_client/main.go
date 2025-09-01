package main

import (
	"fmt"
	"log"
	"net"
)

func main() {
	conn, err := net.Dial("tcp", "localhost:8181")
	if err != nil {
		log.Fatal(err)
	}

	fmt.Println("connected!")

	bytes := make([]byte, 1)

	bytes[0] = 1

	conn.Write(bytes)
	fmt.Println("Send bytes")

	for {
	}

}
