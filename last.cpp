

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include<iostream>

#include<stdio.h>
#include<string>
#include <string.h>
#include <cstdlib>
#include<time.h>

using namespace std;




#define MAX_SEQ 7
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready , no} event_type;


#define MAX_PKT 5 



typedef unsigned int seq_nr;
typedef struct {unsigned char data[MAX_PKT];} packet; 
typedef enum {data, ack, nak} frame_kind; 

typedef struct { 
frame_kind kind; 
seq_nr seq; 
seq_nr ack; 
packet info; 
} frame;

#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0

//start of amgad code

#define PORT 4444

unsigned int server_socket;
unsigned int client_socket;

void StartServer(void){
            std::cout<<"Server started"<<endl;
            server_socket = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in server_address;
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = inet_addr("192.168.1.18");
            server_address.sin_port = htons(PORT);
            bind(server_socket, (sockaddr *)&server_address, sizeof(server_address));
            listen(server_socket, 1);
            std::cout<<"Server is listening on Port: 4444"<<endl;
            client_socket = accept(server_socket, NULL, NULL);
            std::cout<<"Client connected"<<endl;
}

void StartClient(void)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.1.18");
    server_address.sin_port = htons(PORT);
    std::cout<<"connecting to: 192.168.1.18 on Port: 4444"<<endl;
    while (connect(client_socket, (sockaddr *)&server_address, sizeof(server_address)));
    std::cout<<"connected successfully"<<endl;
}




void read_ph(frame *r){
    
    
    read(client_socket, r, sizeof(frame));
    
}

void write_ph(frame *s){
    
    
     write(client_socket, s, sizeof(frame));
    
}

// end of amgad code

int timeout_count = 0;


static bool between(seq_nr a, seq_nr b, seq_nr c)
{
/* Return true if a <= b < c circularly; false otherwise. */
if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
return(true);
else
return(false);
}


class Protocol5
{
private:
    bool full;
    seq_nr next_frame_to_send; 
    seq_nr ack_expected;
    seq_nr frame_expected; 
    frame r,s; 
    packet buffer[MAX_SEQ + 1]; 
    seq_nr nbuffered; 
    seq_nr i; 
    event_type event;
    seq_nr ph_fr1_send;
    seq_nr ph_fr1_rec;
    int number_of_sever;
public:

    Protocol5(int n)
    {
        ack_expected = 0; 
        next_frame_to_send = 0; 
        frame_expected = 0; 
        nbuffered = 0; 
        ph_fr1_send = 0;
        ph_fr1_rec = 0;
        number_of_sever = n;
        enable_network_layer();
    }
    void enable_network_layer() {
        std::cout<<"the data link layer can accept data from network"<<endl;
        full = false;

    }
    void disable_network_layer() {
        std::cout<<"the data link layer can't accept data from network it is full window"<<endl;
        full = true;
    }
    void take_packet(string frame) {
        for (int i = 0; i < 5; i++)
        {
            if (full == false)
            {
                buffer[next_frame_to_send].data[i] = frame[i];
                event = network_layer_ready;
            }
            else
            {
                event = no;
            }
            
        }
        
    }
    void wait_for_event(void) 
    {
        srand(time(0));
        int ran = rand() % 10;
        if (ran <= 9)
        {
            event = frame_arrival;
        }
        else if(ran < 9)
        {
            event = cksum_err;
        }
        else
        {
            event = timeout;
        }
    }

    void send_data()
    {
        s.info = buffer[next_frame_to_send];
        s.seq = next_frame_to_send;
        s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
        std::cout<<"Frame is on the Physical line"<<endl;
        std::cout<<"seq_nr: "<<next_frame_to_send<<" sending ack: "<<s.ack<<endl;
        std::cout<<"the data being sent is :";
        for (int i = 0; i < 5; i++)
        {
            std::cout<<s.info.data[i]<<" ";
        }
        std::cout<<endl;
        write_ph(&s);
        timeout_count++;
    }
    
    
    void from_physical_line()
    {
        std::cout<<"fetching from physical layer a frame"<<endl;        
    }

    void to_network_layer() {
        std::cout<<"network layer is receiving a packet from data link layer"<<endl;
        std::cout<<"the data being received is: ";
        for (int i = 0; i < 5; i++)
        {
            std::cout<<r.info.data[i]<<" ";
        }
        std::cout<<endl;

    }

    void wait_for_data()
    {
        std::cout<<"waiting for data "<<endl;
        read_ph(&r);
        timeout_count--;
        wait_for_event();
        protocol();        
    }

    void check_for_timeout()
    {
        if(timeout_count == 5)
        {
            event = timeout;
            protocol();
        }
    }

    void protocol(void)
    {
        switch(event) {
            case network_layer_ready: 
                std::cout<<"fetching from network layer to data link layer"<<endl;
                nbuffered = nbuffered + 1; 
                send_data();
                inc(next_frame_to_send); 
            break;
            case frame_arrival:
                if (r.seq == frame_expected) {
                    from_physical_line();
                    std::cout<<"expected frame arrived, seq_num: " <<frame_expected<<endl;
                    to_network_layer();
                    inc(frame_expected);
                }
                    /* Ack n implies n − 1, n − 2, etc. Check for this. */
                while (between(ack_expected, r.ack, next_frame_to_send)) {
                    std::cout<<"ack for Frame: "<<ack_expected<<" arrived"<<endl;
                    if (nbuffered - 1 == 7)
                    {
                        enable_network_layer();
                    }
                    nbuffered = nbuffered - 1; 
                    inc(ack_expected);    
                }
            break;
            case cksum_err: 
                std::cout<<"cksum_err happend at server"<<number_of_sever<<endl;
            break;
                
            case timeout:
                std::cout<<endl<<"[!]Timeout for ack: "<<ack_expected<<endl<<endl;
                next_frame_to_send = ack_expected; 
                for (i = 1; i <= nbuffered; i++) {
                    send_data();
                    inc(next_frame_to_send); 
                }
            }
        if (nbuffered < MAX_SEQ)
            int x = 0;
        else
            disable_network_layer();
        }
};


int main()
{
    StartServer();
    string packet_network;
    int operation = 1;
    Protocol5 server1(1);
    while (1)
    {
        std::cout<<endl;
        std::cout<<"specify mode (1: enter a packet to send) (2: wait for physical line)"<<endl;
        cin>>operation;
        
        if (operation == 1)
        {
            do
            {
                std::cout<<"input a Packet of 5 chars"<<endl;
                cin>>packet_network;
                server1.take_packet(packet_network);
                cin.ignore();
                server1.protocol();
                server1.check_for_timeout();    
            }while(packet_network.length() != 5);



    
        }
        else
        {
            
            server1.wait_for_data();
        }    

    }
}

