/*
 * HoverDrone Testing Sketch : Receiver Utilities
 *
 * This file is part of HoverDrone.
 *
 * HoverDrone is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoverDrone is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoverDrone.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * Author: Joseph Monti <joe.monti@gmail.com>
 * Copyright (c) 2013 Joseph Monti All Rights Reserved, http://joemonti.org/
 */

const int NUM_CHANNELS = 6;
const int MIN_IN_PULSE_WIDTH = 750;
const int MAX_IN_PULSE_WIDTH = 2250;
const int SYNC_GAP_LEN = 3000;
const int INITIALIZE_CYCLES = 10;

const long ZEROING_TIME = 1000 * 1000 * 5;
const float ZERO_BUFFER = 0.005;

const int MAX_GAP = 1920;
const int MIN_GAP = 1110;

const int RECEIVER_STATE_PENDING = 0;
const int RECEIVER_STATE_INITIALIZING = 1;
const int RECEIVER_STATE_ZEROING = 2;
const int RECEIVER_STATE_READY = 3;

volatile unsigned long last_micros;
volatile int initializedCount;
volatile int receiverState;
volatile int channel;
volatile int pulsesTmp[NUM_CHANNELS];
volatile int pulses[NUM_CHANNELS];

volatile long zeroReadyTime;
volatile boolean zeroed;
volatile int pulsesMinZero[NUM_CHANNELS];
volatile int pulsesMaxZero[NUM_CHANNELS];
volatile int pulsesCentValue[NUM_CHANNELS];
volatile int pulsesMin[NUM_CHANNELS];
volatile int pulsesMax[NUM_CHANNELS];

void receiver_setup( ) {
  last_micros = 0;
  initializedCount = 0;
  receiverState = RECEIVER_STATE_PENDING;
  channel = 0;
  
  for ( int i = 0; i < NUM_CHANNELS; i++ ) {
    pulsesMinZero[i] = -1;
    pulsesMaxZero[i] = -1;
  }
  
  attachInterrupt( RECEIVER_PIN, receiver_interrupt, RISING);
}

boolean receiver_ready() {
  return receiverState == RECEIVER_STATE_READY;
}

void receiver_interrupt( ) {
  unsigned long current_micros = micros();
  unsigned long gap = current_micros - last_micros;
  
  if ( gap >= SYNC_GAP_LEN ) {
    if ( receiverState == RECEIVER_STATE_READY ) {
      if ( channel != NUM_CHANNELS ) {
        receiverState = RECEIVER_STATE_INITIALIZING;
      }
      
      for ( int i = 0; i< NUM_CHANNELS; i++ ) {
        pulses[i] = pulsesTmp[i];
      }
    } else if ( receiverState == RECEIVER_STATE_ZEROING ) {
      if ( channel != NUM_CHANNELS ) {
        receiverState = RECEIVER_STATE_INITIALIZING;
      } else {
        for ( int i = 0; i < NUM_CHANNELS; i++ ) {
          if ( pulsesMinZero[i] == -1 || pulsesTmp[i] < pulsesMinZero[i] ) {
            pulsesMinZero[i] = pulsesTmp[i];
          }
          if ( pulsesMaxZero[i] == -1 || pulsesTmp[i] > pulsesMaxZero[i] ) {
            pulsesMaxZero[i] = pulses[i];
          }
        }
        
        if ( current_micros >= zeroReadyTime ) {
          receiverState = RECEIVER_STATE_READY;
          
          for ( int i = 0; i < NUM_CHANNELS; i++ ) {
            int cent = ( pulsesMinZero[i] + pulsesMaxZero[i] ) / 2;
            pulsesMinZero[i] -= ( pulsesMinZero[i] * ZERO_BUFFER );
            pulsesMaxZero[i] += ( pulsesMaxZero[i] * ZERO_BUFFER );
            int range = min( MAX_GAP - cent, cent - MIN_GAP );
            pulsesMin[i] = cent - range;
            pulsesMax[i] = cent + range;
            pulsesCentValue[i] = map( cent, pulsesMin[i], pulsesMax[i], 0, 200 );
          }
        }
      }
    } else if ( receiverState == RECEIVER_STATE_INITIALIZING ) {
      initializedCount++;
      if ( initializedCount >= INITIALIZE_CYCLES ) {
        if ( !zeroed ) {
          zeroReadyTime = current_micros + ZEROING_TIME;
          receiverState = RECEIVER_STATE_ZEROING;
        } else {
          receiverState = RECEIVER_STATE_READY;
        }
      }
    } else {
      receiverState = RECEIVER_STATE_INITIALIZING;
      initializedCount = 0;
    }
    channel = 0;
  } else if ( channel < NUM_CHANNELS ) {
    if ( gap >= MIN_IN_PULSE_WIDTH && gap <= MAX_IN_PULSE_WIDTH ) {
      pulsesTmp[channel] = gap;
      channel++;
    } else if ( receiverState == RECEIVER_STATE_READY || receiverState == RECEIVER_STATE_ZEROING ) {
      receiverState = RECEIVER_STATE_INITIALIZING;
      channel = 0;
    }
  }
  
  last_micros = current_micros;
}

int receiver_get_state() { 
  return receiverState;
}

int receiver_get_value( int c ) {
  if ( receiverState == RECEIVER_STATE_READY ) {
    int pulse = pulses[c];
    if ( pulse >= pulsesMinZero[c] && pulse <= pulsesMaxZero[c] ) {
      return 0;
    }
    int value = map( pulse, pulsesMin[c], pulsesMax[c], 0, 200 ) - 
                    pulsesCentValue[c];
    return max( -100, min( value, 100 ) );
  } else {
    return -1;
  }
}

int receiver_get_pulse( int c ) {
  if ( receiverState == RECEIVER_STATE_READY ) {
    return pulses[c];
  } else {
    return -1;
  }
}
