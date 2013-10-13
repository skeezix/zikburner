
#ifndef h_serial_server_h
#define h_serial_server_h

#define MAXDATASIZE 256

void serial_setup ( void );

void serial_loop_forever ( void );

void serial_echo_loop_forever ( void );

// internal..
typedef enum {
  ss_ready = 0,
  ss_cmd_build,
  ss_ohai,
  ss_echo,
  ss_receive,
  ss_burn,
  ss_dump,
} serial_state_e;

#endif
