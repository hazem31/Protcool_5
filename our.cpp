

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


static bool between(seq_nr a, seq_nr b, seq_nr c)
{
/* Return true if a <= b < c circularly; false otherwise. */
if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
return(true);
else
return(false);
}


// static void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[ ])
// {
// /* Construct and send a data frame. */
// frame s; /* scratch variable */
// s.info = buffer[frame_nr]; /* insert packet into frame */
// s.seq = frame_nr; /* insert sequence number into frame */
// s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1); /* piggyback ack */
// //to_physical_layer(&s); /* transmit the frame */
// //start timer(frame nr); /* start the timer running */
// }


frame physical_line,physical_line2;


class Protocol5
{
private:
    bool full;
    seq_nr next_frame_to_send; /* MAX SEQ > 1; used for outbound stream */
    seq_nr ack_expected; /* oldest frame as yet unacknowledged */
    seq_nr frame_expected; /* next frame expected on inbound stream */
    frame r; /* scratch variable */
    packet buffer[MAX_SEQ + 1]; /* buffers for the outbound stream */
    seq_nr nbuffered; /* number of output buffers currently in use */
    seq_nr i; /* used to index into the buffer array */
    event_type event;
    // laterrrrrrrrrrrrrrrrrrrrr enable_network_layer(); /* allow network layer ready events */
public:

    Protocol5()
    {
        ack_expected = 0; /* next ack expected inbound */
        next_frame_to_send = 0; /* next frame going out */
        frame_expected = 0; /* number of frame expected inbound */
        nbuffered = 0; /* initially no packets are buffered */
        enable_network_layer();
    }
    void enable_network_layer() {
        cout<<"the data link layer can accept data from network"<<endl;
        full = false;

    }
    void disable_network_layer() {
        cout<<"the data link layer can't accept data from network it is full window"<<endl;
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
        cout<<"ran is"<<ran<<endl;
        if (ran < 8)
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
    physical_line.info = buffer[next_frame_to_send]; /* insert packet into frame */
    physical_line.seq = next_frame_to_send; /* insert sequence number into frame */
    physical_line.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1); /* piggyback ack */
    cout<<"Frame is on the Physical line"<<endl;
    cout<<"the data being sent is :";
    for (int i = 0; i < 5; i++)
    {
        cout<<physical_line.info.data[i]<<" ";
    }
    cout<<endl;

    //start timer(frame nr); /* start the timer running */
    }

    void from_physical_line()
    {
        r.info = physical_line.info;
        r.seq = physical_line.seq;
        r.ack = physical_line.ack;
        cout<<"fetching from physiacl layer a frame"<<endl;
    }

    void to_network_layer() {
        cout<<"network layer is reciving a packet from data link layer"<<endl;
        cout<<"the data being sent is :";
        for (int i = 0; i < 5; i++)
        {
            cout<<r.info.data[i]<<" ";
        }
        cout<<endl;

    }

    void protocol(void)
    {
        //wait_for_event();
        switch(event) {
            case network_layer_ready: /* the network layer has a packet to send */
                nbuffered = nbuffered + 1; /* expand the sender’s window */
                send_data();/* transmit the frame */
                inc(next_frame_to_send); /* advance sender’s upper window edge */
                break;
                case frame_arrival: /* a data or control frame has arrived */
                    from_physical_line(); /* get incoming frame from physical layer */
                    if (r.seq == frame_expected) {
                        /* Frames are accepted only in order. */
                        to_network_layer(); /* pass packet to network layer */
                        inc(frame_expected); /* advance lower edge of receiver’s window */
                    }
                    /* Ack n implies n − 1, n − 2, etc. Check for this. */
                    while (between(ack_expected, r.ack, next_frame_to_send)) {
                        nbuffered = nbuffered - 1; 
                        // laterrrrrrrrrrrrrrrrrrrrrrrr stop_timer(ack_expected); /* frame arrived intact; stop timer */
                        inc(ack_expected); /* contract sender’s window */   
                    }
                break;
                case cksum_err: 
                    cout<<"cksum_err happend:"<<endl;
                break; /* just ignore bad frames */
                
                case timeout: /* trouble; retransmit all outstanding frames */
                    cout<<"Time out happend for ack:"<<ack_expected<<endl;
                    next_frame_to_send = ack_expected; /* start retransmitting here */
                    for (i = 1; i <= nbuffered; i++) {
                        send_data();/* resend frame */
                        inc(next_frame_to_send); /* prepare to send the next one */
                    }
            }
            if (nbuffered < MAX_SEQ)
            //enable_network_layer();
            int x = 0;
            else
            disable_network_layer();
        }
};


int main()
{
    string stop = "n";
    int server_number = 1;
    string packet_network;  
    Protocol5 server1,server2;
    while (1)
    {
        do
        {
            cout<<"input a Packet of 5 chars"<<endl;
            cin>>packet_network;    
        } while (packet_network.length() != 5);

        do
        {
            cout<<"input server number 1 or 2"<<endl;
            cin>>server_number;
        } while (server_number != 1 && server_number != 2);
        
        if (server_number == 1)
        {
            server1.take_packet(packet_network);
            server1.protocol();
            server2.wait_for_event();
            server2.protocol();
        }
        else
        {
            server2.take_packet(packet_network);
            server2.protocol();
        }
        
        
        
        cout<<"if you want to exist enter y if want to continue then enter n"<<endl;
        cin>>stop;
        if (stop == "y")
        {
            break;
        }
        

    }
      

    
    cout<<"done"<<endl;
	return 0;

}
