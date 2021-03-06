#include <string.h>
#include <pthread.h>

#include "minunit.h"
#include <cvlc/dbg.h>
#include <link/link.h>
#include <cvlc/byte_layer.h>
#include <cvlc/bstrlib.h>
#include <cvlc/packet_layer.h>
#include <cvlc/file_layer.h>

void *test_init()
{
  int rc = 0;

  //Init link layer
  rc = init_link();
  mu_assert(rc == 0, "Failed to init link layer.");

  //Init packet layer
  rc = init_packet_layer();
  mu_assert(rc == 0, "Failed to init packet layer.");

  return NULL;
}

char bits_out[8] = {0};
char bits_in[8] = {0};

void *send_bits(void *arg)
{
  int i;
  for(i = 0;i < 8;i++) send_bit(bits_out[i]);
  return NULL;
}

void *get_bits(void *arg)
{
  int i;
  for(i = 0;i < 8;i++) bits_in[i] = get_bit();
  return NULL;
}


char *test_bit_layer()
{
  int rc = 0;

  //Set up sequence to send
  bits_out[0] = 1;
  bits_out[2] = 1;
  bits_out[7] = 1;

  //Create threads for sending and receiving
  pthread_t rx_tid;
  pthread_t tx_tid;

  pthread_attr_t rx_attr;
  pthread_attr_t tx_attr;

  rc = pthread_attr_init(&rx_attr);
  mu_assert(rc == 0, "Failed to init rx thread.");

  rc = pthread_attr_init(&tx_attr);
  mu_assert(rc == 0, "Failed to init tx thread.");

  pthread_create(&rx_tid, &rx_attr, send_bits, "");
  pthread_create(&tx_tid, &tx_attr, get_bits, "");

  pthread_join(rx_tid, NULL);
  pthread_join(tx_tid, NULL);

  rc = strncmp(bits_in, bits_out, 8);
  mu_assert(rc == 0, "Bits coming in doesn't match what was sent.");

  return NULL;
}

char byte_out = 'c';
char byte_in = 0;

void *send_bytes(void *arg)
{
  send_byte(byte_out);
  return NULL;
}

void *get_bytes(void *arg)
{
  byte_in = get_byte();
  return NULL;
}

char *test_byte_layer()
{
  int rc = 0;

  //Create threads for sending and receiving
  pthread_t rx_tid;
  pthread_t tx_tid;

  pthread_attr_t rx_attr;
  pthread_attr_t tx_attr;

  rc = pthread_attr_init(&rx_attr);
  mu_assert(rc == 0, "Failed to init rx thread.");

  rc = pthread_attr_init(&tx_attr);
  mu_assert(rc == 0, "Failed to init tx thread.");

  pthread_create(&rx_tid, &rx_attr, send_bytes, "");
  pthread_create(&tx_tid, &tx_attr, get_bytes, "");

  pthread_join(rx_tid, NULL);
  pthread_join(tx_tid, NULL);

  mu_assert(byte_in == byte_out, "Byte recieved doesn't match byte sent.");

  return NULL;
}

bstring packet_in;
bstring packet_out;

void *send_packets()
{
  int rc = 0;
  rc = send_packet(packet_out);
  mu_assert(rc == 0, "Error in send_packet");
  return NULL;
}

void *get_packets()
{
  packet_in = get_data_frame();
  mu_assert(packet_in, "Error in get_packet");
  return NULL;
}

/* Test sending a regular, correct packet.
 */
char *test_packet()
{
  int rc = 0;

  //Set up a packet
  bstring payload = bfromcstr("test packet");
  packet_out = create_data_frame(payload);

  //Create threads for sending and receiving
  pthread_t rx_tid;
  pthread_t tx_tid;

  pthread_attr_t rx_attr;
  pthread_attr_t tx_attr;

  rc = pthread_attr_init(&rx_attr);
  mu_assert(rc == 0, "Failed to init rx thread.");

  rc = pthread_attr_init(&tx_attr);
  mu_assert(rc == 0, "Failed to init tx thread.");

  pthread_create(&rx_tid, &rx_attr, send_packets, "");
  pthread_create(&tx_tid, &tx_attr, get_packets, "");

  pthread_join(rx_tid, NULL);
  pthread_join(tx_tid, NULL);

  rc = biseq(packet_in, packet_out);
  mu_assert(rc != 0, "Packet sent isn't equal to packet recieved.");
  
  return NULL;
}

/* Test sending a packet with some unwanted padding prepended.
 */
char *test_prepended_packet()
{
  int rc = 0;

  //Set up a packet
  bstring payload = bfromcstr("test packet");
  bstring packet = create_data_frame(payload);
  bstring padding = bfromcstr("a");
  bconcat(padding, packet);
  packet_out = padding;

  //Create threads for sending and receiving
  pthread_t rx_tid;
  pthread_t tx_tid;

  pthread_attr_t rx_attr;
  pthread_attr_t tx_attr;

  rc = pthread_attr_init(&rx_attr);
  mu_assert(rc == 0, "Failed to init rx thread.");

  rc = pthread_attr_init(&tx_attr);
  mu_assert(rc == 0, "Failed to init tx thread.");

  pthread_create(&rx_tid, &rx_attr, send_packets, "");
  pthread_create(&tx_tid, &tx_attr, get_packets, "");

  pthread_join(rx_tid, NULL);
  pthread_join(tx_tid, NULL);

  rc = biseq(packet_in, packet_out);
  mu_assert(rc != 0, "Packet sent isn't equal to packet recieved.");
  
  return NULL;
}

/* Send a damaged packet to check if the error detection works.
 */
char *test_incorrect_packet()
{
  return NULL;
}

char *in_file = "test_file.txt";
char *out_file = "test_out_file.txt";

void *send_files()
{
  send_file(in_file);
  return NULL;
}

void *get_files()
{
  get_file(out_file);
  return NULL;
}

/* Send a file consisting of several packets.
 */
char *test_file()
{
  int rc = 0;

  //Set up a packet

  //Create threads for sending and receiving
  pthread_t rx_tid;
  pthread_t tx_tid;

  pthread_attr_t rx_attr;
  pthread_attr_t tx_attr;

  rc = pthread_attr_init(&rx_attr);
  mu_assert(rc == 0, "Failed to init rx thread.");

  rc = pthread_attr_init(&tx_attr);
  mu_assert(rc == 0, "Failed to init tx thread.");

  pthread_create(&rx_tid, &rx_attr, send_files, "");
  pthread_create(&tx_tid, &tx_attr, get_files, "");

  pthread_join(rx_tid, NULL);
  pthread_join(tx_tid, NULL);
  
  return NULL;
}


char *all_tests()
{
  mu_suite_start();

  mu_run_test(test_init);
  mu_run_test(test_bit_layer);
  mu_run_test(test_byte_layer);
  mu_run_test(test_packet);
  mu_run_test(test_prepended_packet);
  mu_run_test(test_file);

  return NULL;
}

RUN_TESTS(all_tests);
