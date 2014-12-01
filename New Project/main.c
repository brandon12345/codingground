//------------------------------------------------------------------------------------------------
// TMR0 RPM PULSE GENERATOR TABLE CREATOR ( WINDOWS C CONSOLE RUN CODE )
//------------------------------------------------------------------------------------------------
 
#include <stdio.h>
 
#define osc  64000000ul // Mhz osc value
#define instruction_clock_cycle  4ul // number of osc cycles to complete one instruction
#define number_of_prescalers  9ul // amount of prescalers in tmr0
#define number_of_preloads  65536ul // amount of preload values
#define number_of_defined_rpms  6400ul // how many integer rpms are covered
 
const unsigned int prescalers[ number_of_prescalers ] = { 1,2,4,8,16,32,64,128,256 };// tmr0 prescaler values
unsigned int prescaler_index, preload_index;// provides linking
unsigned long rpm;// rpm literal, starts with 1 rpm
float prescaler_match_high, prescaler_match_low;// stores current match
float preload_match_high, preload_match_low;// stores current match
unsigned long i,j;// loop variables
FILE *fp;
 
//------------------------------------------------------------------------------------------------
// string functions
//------------------------------------------------------------------------------------------------
#define array_size  64
 
float float_whole_to_float_fraction_with_modifier(float value, char digits){
  // used in string_to_float() only, this uses digits counted in calling funtion
  // so .0001, .1000, .1010 would all print correctly on the screen
  char i;
  for(i=0;i<digits;i++){
    value/=10;
  }
  return value;
}
 
char* number_to_string( long n ){
  static char temp_string[array_size];
  char i=0;
  char t=0;
  char bypass=0;
  char temp_cnt=0;
  signed long m=1000000000;
 
  if(n<0){temp_string[temp_cnt++]='-';n=0-n;}
  for(i=0;i<10;i++){
    t=(n/m)%10;
    if(t>0) bypass=1;
    if(bypass)temp_string[temp_cnt++]=(t+48);
    m/=10;
   }
  if(bypass==0)temp_string[temp_cnt++]=(t+48);
  temp_string[temp_cnt]=0;
  return temp_string;// returning pointer to temp string
}
 
char* float_to_string( float floater ){
  static char temp_string[array_size];
  char temp_cnt=0;
  char i=0;
  char t=0;
  char bypass=0;
  long m=1000000000;
  long longer;
 
  if(floater<0){
    temp_string[temp_cnt++]='-';
    floater=0-floater;// making number negative for math
  }
  longer = (long)floater;// converting float to long
  for(i=0;i<10;i++){
    t=(longer/m)%10;
    if(t>0) bypass=1;
    if(bypass)temp_string[temp_cnt++]=(t+48);
    m/=10;
  }
  if(bypass==0) temp_string[temp_cnt++]=(t+48);
  floater-=longer;// difference between float and whole number
  floater*=10000;// to get 4 digits to right of decimal
  longer=(long)floater;// converting back to long
  temp_string[temp_cnt++]=('.');
  temp_string[temp_cnt++] = ((longer / 1000) % 10) + 48;
  temp_string[temp_cnt++] = ((longer / 100) % 10) + 48;
  temp_string[temp_cnt++] = ((longer / 10) % 10) + 48;
  temp_string[temp_cnt++] = (longer % 10) + 48;
  temp_string[temp_cnt]=0;
  return temp_string;
}
 
//------------------------------------------------------------------------------------------------
// print functions
//------------------------------------------------------------------------------------------------
void print_string( char *p ){
  fprintf( fp, p );
}
 
void new_line( char how_many ){
  char i;
  for( i=0; i<how_many; i++ ){
    print_string( "\n" );
  }
}
 
void spaces( char how_many ){
  char i;
  for( i=0; i<how_many; i++ ){
    print_string( " " );
  }
}
 
void print_number( long x ){
  char *p;
  p = number_to_string( x );
  print_string( p );
}
 
void print_float( float x ){
  char *p;
  p = float_to_string( x );
  print_string( p );
}
 
//------------------------------------------------------------------------------------------------
// calculation functions
//------------------------------------------------------------------------------------------------
//float calc_tmr0_freq( unsigned int prescaler, unsigned long preload );
//float calc_rpm_to_hz( float rpm );
//float calc_hz_to_rpm( float hz );
//float calc_rpm_ms_period( float rpm );
//void print_result( unsigned int index, long prescaler, long preload );
 
 
float calc_tmr0_freq( unsigned int prescaler, unsigned long preload ){
  //return ( ( osc / instruction_clock_cycle ) / prescaler )  / ( 65535 - preload );
  float hz;
  hz = osc / instruction_clock_cycle;
  hz = hz / prescaler;
  preload = (65536 - preload );
  hz = hz / preload;
  return hz;
}
 
float calc_rpm_to_hz( float rpm ){
  return (float)rpm / 60.0;
}
 
float calc_hz_to_rpm( float hz ){
  return hz * 60.0;
}
 
float calc_rpm_ms_period( float rpm ){
  float hz;
  hz = calc_rpm_to_hz( rpm );
  return 1000.00 / hz;
}
 
 
void print_rpm_result( unsigned int index, long prescaler, long preload ){
  float rpm, hz;
  hz = calc_tmr0_freq( prescaler, preload );
  rpm = calc_hz_to_rpm( hz );
  rpm = rpm / 2;
  new_line( 1 );  print_number( index );
  spaces( 4 );    print_string( "closest rpm: " );  print_float( rpm );
  spaces( 4 );  print_string( "prescaler: " ); print_number( prescaler );
  spaces( 4 );  print_string( "preload: " ); print_number( preload );
  spaces( 4 );  print_string( "tmr hz: " ); print_float( hz );
}
 
 
 
 
 
 
#define show_list_format        0 // this displays list of all results for ranges
#define show_rpm_results_format 1 // displays the closest tmr0 matches for rpm
#define prescaler_table_format  2 // creates code to drop into table
#define preload_table_format    3 // creates code to drop into table
 
int main( void ){
  #define output_result  prescaler_table_format
  #define table_values_per_line 10
  #define first_rpm  30
  #define last_rpm  6400
  #define rpm_increments  1
 
  unsigned long number_of_results = 0;
 
  float tmr0_hz;// calculated tmr0 result from osc/4/prescaler/(65536-preload)
  float rpm_hz;// crank frequency of rpm
  float rpm_hz_match_high, rpm_hz_match_low;
  float freq_match_high, freq_match_low;// logs
 
  unsigned int table_tracker = 0;// maintains entries per line as set with #defien table_values_per_line
 
  fp = fopen( "C:\\Documents and Settings\\heather springett\\My Documents\\table_results", "w" );
  if( fp == NULL ) return 1;
 
  new_line( 4 );
 
  rpm = first_rpm;// starts with 1 rpm
 
  while( rpm < ( last_rpm + 1 )  ){
    rpm_hz = calc_rpm_to_hz( rpm * 2 );// because timer will toggle twice which will cut output hz in half
    rpm_hz_match_high = 1000;// starts high match at hz( rpm ) times 10
    rpm_hz_match_low =  0;// starts low search match at 0
 
        printf( "\n" );
        printf( "%d",rpm ); 
    for( i=0; i<number_of_prescalers; i++ ){
      for( j=0; j<number_of_preloads; j++ ){
        tmr0_hz = calc_tmr0_freq( prescalers[ i ], j );
        if( ( tmr0_hz >= rpm_hz ) && ( tmr0_hz < rpm_hz_match_high ) ){
          prescaler_match_high = prescalers[ i ];
          preload_match_high = j;
          rpm_hz_match_high = tmr0_hz;
        }
        if( ( tmr0_hz <= rpm_hz ) && ( tmr0_hz > rpm_hz_match_low ) ){
          prescaler_match_low = prescalers[ i ];
          preload_match_low = j;
          rpm_hz_match_low = tmr0_hz;
        }
      }
    }
 
    #if output_result == result_list_format
      number_of_results++;
      if( rpm_hz_match_high >= rpm_hz_match_low )  print_rpm_result( rpm, prescaler_match_high, preload_match_high );
      else print_rpm_result( rpm, prescaler_match_high, preload_match_high );
    #endif
 
    #if output_result == prescaler_table_format
      number_of_results++;
      table_tracker++;
      if( rpm_hz_match_high >= rpm_hz_match_low ) print_number( prescaler_match_high );
      else print_number( prescaler_match_low );
      print_string( "," );
      if( table_tracker == table_values_per_line ){
        table_tracker = 0;
        new_line( 1 );
      }
    #endif
 
    #if output_result == preload_table_format
      number_of_results++;
      table_tracker++;
      if( rpm_hz_match_high >= rpm_hz_match_low ) print_number( preload_match_high );
      else print_number( preload_match_low );
      print_string( "," );
      if( table_tracker == table_values_per_line ){
        table_tracker = 0;
        new_line( 1 );
      }
    #endif
 
    #if output_result == show_rpm_results_format
      number_of_results++;
      print_string( "closest rpm for: ");  print_number( rpm ); spaces( 4 );
      if( rpm_hz_match_high >= rpm_hz_match_low ){
        float closest_rpm;
        closest_rpm = calc_hz_to_rpm( rpm_hz_match_high );
        closest_rpm /= 2;
        print_float( closest_rpm );
      }
      else{
        float closest_rpm;
        closest_rpm = calc_hz_to_rpm( rpm_hz_match_low );
        closest_rpm /= 2;
        print_float( closest_rpm );
      }
      new_line( 1 );
    #endif
 
    rpm+=rpm_increments;
  }
 
  new_line( 3 );
  print_string( "number of results: " ); print_number( number_of_results );
  new_line( 3 );
  fclose( fp );
}
