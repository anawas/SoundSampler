/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: andreaswassmer
 *
 * Created on 22. Mai 2017, 09:08
 */

#include <cstdlib>
#include <sys/mman.h>
#include <limits.h>

#include "sensorboardconnector.h"
#include "mcp3008Spi.h"
#include "wavfile.h"

using namespace std;

#define FILEPATH "/tmp/mmapped.bin"
#define SAMPLE_DURATION 20
#define N_SAMPLES (WAVFILE_SAMPLES_PER_SECOND * SAMPLE_DURATION)
#define FILESIZE (N_SAMPLES * sizeof(short)) 

/*
 * 
 */
int main(int argc, char** argv) {
    mcp3008Spi a2d("/dev/spidev0.0", SPI_MODE_0, 1000000, 8);

    FILE *f = wavfile_open("sound.wav");
    short *buffer;
    int fd, result;
    int sample_freq = (int)((1./WAVFILE_SAMPLES_PER_SECOND) * 1e6 / 2.0);

    printf("Vorbereiten zum Sampling mit %d kHZ\n", WAVFILE_SAMPLES_PER_SECOND);
    printf("Dauer pro Sample: %d ms\n", sample_freq);
    printf("Laenge des Samples: %d s\n", SAMPLE_DURATION);

    fd = fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
      if (fd == -1) {
	perror("Error opening file for writing");
	exit(EXIT_FAILURE);
    }
    
    /* Stretch the file size to the size of the (mmapped) array of ints
     */
    result = lseek(fd, FILESIZE-1, SEEK_SET);
    if (result == -1) {
	close(fd);
	perror("Error calling lseek() to 'stretch' the file");
	exit(EXIT_FAILURE);
    }
  
       /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched 
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */
    result = write(fd, "", 1);
    if (result != 1) {
	close(fd);
	perror("Error writing last byte of the file");
	exit(EXIT_FAILURE);
    }

    /* Now the file is ready to be mmapped.
     */
    buffer = (short *)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
	close(fd);
	perror("Error mmapping the file");
	exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < N_SAMPLES; i++) {
        int v = sampleSound(a2d, sample_freq, 3);
        buffer[i] = (short)v;
    }
    
    short max = 0;
    short min = 10000;
    
    for (int i = 0; i < N_SAMPLES; i++) {
        if (buffer[i] < min) min = buffer[i];
        if (buffer[i] > max) max = buffer[i];
    }
    
    float scale = 16000.0 / max;
    printf("min = %hd\n", min);
    printf("max = %hd --> scale = %f\n", max, scale);

    for (int i = 0; i < N_SAMPLES; i++) {
        buffer[i] -= min;
        buffer[i] *= scale;
    }
    
    wavfile_write(f, buffer, N_SAMPLES);
    wavfile_close(f);
    
      /* Don't forget to free the mmapped memory
     */
    if (munmap(buffer, FILESIZE) == -1) {
	perror("Error un-mmapping the file");
	/* Decide here whether to close(fd) and exit() or not. Depends... */
    }

    /* Un-mmaping doesn't close the file, so we still need to do that.
     */
    close(fd);
    
    return 0;
}

