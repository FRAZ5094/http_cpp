package main

import (
	"encoding/binary"
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

	bytes := make([]byte, 2)

	binary.BigEndian.PutUint16(bytes, 325)

	fmt.Printf("%d, %d", bytes[0], bytes[1])

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
