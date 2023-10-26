/* 
 * pttkey - keyboard PTT reader
 *  
 * Copyright (C) 2023  Resilience Theatre
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "log.h"
#include "ini.h"

/* Write shared memory status for QT ui */
void s_write(int state)
{
    const int SIZE = 4096;
    const char *name = "os";
    const char *message_0 = "down";     // state == 0
    const char *message_1 = "up";       // state == 1
    int shm_fd;
    void *ptr;
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    /* write to the shared memory object */
    if (state == 0)
    {
        sprintf(ptr, "%s", message_0);
        ptr += strlen(message_0);
    }
    if (state == 1)
    {
        sprintf(ptr, "%s", message_1);
        ptr += strlen(message_1);
    }
}

void usage()
{
    log_info("[%d] pttkey", getpid());
    log_info("[%d] Usage: -i [ini file]", getpid());
    log_info("[%d] Usage: -t train mode filtered ", getpid());
    log_info("[%d] Usage: -T train mode unfiltered ", getpid());
    log_info("[%d] Use 'evtest' to checkout your keyboard device ", getpid());
}

int main(int argc, char *argv[])
{
    int rcode = 0;
    char keyboard_name[256] = "Unknown";
    char *keyboard_device = NULL;
    char *ptt_down_command = NULL;
    char *ptt_up_command = NULL;
    char *ini_file = NULL;
    int state = 0;
    int c;
    int trainmode = 0;
    char *ptt_down_type, *ptt_down_code, *ptt_down_value;
    char *ptt_up_type, *ptt_up_code, *ptt_up_value;

    /* Command line options */
    while ((c = getopt(argc, argv, "i:htT")) != -1)
        switch (c)
        {
        case 'i':
            ini_file = optarg;
            break;
        case 'h':
            usage();
            return 1;
        case 't':
            trainmode = 1;
            break;
        case 'T':
            trainmode = 2;
            break;
            break;
        default:
            break;
        }
    if (ini_file == NULL)
    {
        usage();
        return -1;
    }
    ini_t *config = ini_load(ini_file);
    
    /* Read ini-file */
    ini_sget(config, "pttkey", "keyboard_device", NULL, &keyboard_device);
    ini_sget(config, "pttkey", "ptt_down_command", NULL, &ptt_down_command);
    ini_sget(config, "pttkey", "ptt_up_command", NULL, &ptt_up_command);

    ini_sget(config, "pttkey", "ptt_down_type", NULL, &ptt_down_type);
    ini_sget(config, "pttkey", "ptt_down_code", NULL, &ptt_down_code);
    ini_sget(config, "pttkey", "ptt_down_value", NULL, &ptt_down_value);

    ini_sget(config, "pttkey", "ptt_up_type", NULL, &ptt_up_type);
    ini_sget(config, "pttkey", "ptt_up_code", NULL, &ptt_up_code);
    ini_sget(config, "pttkey", "ptt_up_value", NULL, &ptt_up_value);

    log_info("[%d] PTT Down: %d %d %d", getpid(), atoi(ptt_down_type), atoi(ptt_down_code), atoi(ptt_down_value));
    log_info("[%d] PTT Up: %d %d %d", getpid(), atoi(ptt_up_type), atoi(ptt_up_code), atoi(ptt_up_value));

    /* Open keyboard */
    int keyboard_fd = open(keyboard_device, O_RDONLY | O_NONBLOCK);
    if (keyboard_fd == -1)
    {
        log_error("[%d] Failed to open keyboard.", getpid());
        exit(1);
    }
    rcode = ioctl(keyboard_fd, EVIOCGNAME(sizeof(keyboard_name)), keyboard_name);
    log_info("[%d] Reading from: %s (%d)", getpid(), keyboard_name, rcode);
    struct input_event keyboard_event;

    while (1)
    {

        if (read(keyboard_fd, &keyboard_event, sizeof(keyboard_event)) != -1)
        {

            if (trainmode == 0)
            {
                if (state == 0 && keyboard_event.type == atoi(ptt_down_type)
                    && keyboard_event.code == atoi(ptt_down_code) && keyboard_event.value == atoi(ptt_down_value))
                {
                    log_info("[%d] PTT down", getpid());
                    system(ptt_down_command);
                    state = 1;
                    // s_write(0);
                }
                if (state == 1 && keyboard_event.type == atoi(ptt_up_type) && keyboard_event.code == atoi(ptt_up_code)
                    && keyboard_event.value == atoi(ptt_up_value))
                {
                    log_info("[%d] PTT up", getpid());
                    system(ptt_up_command);
                    state = 0;
                    // s_write(1);
                }
            }
            if (trainmode == 1)
            {
                if (keyboard_event.type == 4 && keyboard_event.code == 4)
                    printf("Keyboard event: type %d code %d value %d  \n", keyboard_event.type, keyboard_event.code,
                           keyboard_event.value);
            }
            if (trainmode == 2)
            {
                printf("Keyboard event: type %d code %d value %d  \n", keyboard_event.type, keyboard_event.code,
                       keyboard_event.value);
            }
        }
        sleep(0.1);
    }
    log_info("[%d] Exiting", getpid());
    rcode = ioctl(keyboard_fd, EVIOCGRAB, 1);
    close(keyboard_fd);
    return 0;
}
