package main

import (
	"fmt"
	"log"
	"net"
	"time"
)

func main() {
	conn, err := net.Dial("tcp", "localhost:8181")
	if err != nil {
		log.Fatal(err)
	}

	fmt.Println("connected!")

	bytes := make([]byte, 1)

	bytes[0] = 1

	for {
		n, err := conn.Write(bytes)
		if err != nil {
			fmt.Println("Write error:", err)
			break
		}
		fmt.Println("Sent bytes:", n)
		time.Sleep(100 * time.Millisecond)
	}

}
