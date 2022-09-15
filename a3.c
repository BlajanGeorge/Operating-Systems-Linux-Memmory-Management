#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define FIFO_WRITE "RESP_PIPE_98237"
#define FIFO_READ "REQ_PIPE_98237"


int main()
{
    if(mkfifo(FIFO_WRITE,0600) != 0)
    {
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }

    int fd_read = -1;
    int fd_write = -1;

    fd_read = open(FIFO_READ,O_RDONLY);
    if(fd_read == -1)
    {
        printf("ERROR\ncannot open the request pipe\n");
        unlink(FIFO_WRITE);
        return 1;
    }

    fd_write = open(FIFO_WRITE,O_WRONLY);

    if(fd_write == -1)
    {
        printf("ERROR\ncannot open the response pipe\n");
        close(fd_read);
        unlink(FIFO_WRITE);
        return 1;
    }
    int dim=7;

    if(write(fd_write,&dim,1) != 1)
    {
        printf("ERROR\n");
        return 1;
    }
    if(write(fd_write,"CONNECT",7) != 7)
    {
        printf("ERROR\n");
        return 1;
    }
    printf("SUCCESS\n");

    char* data = NULL;  //aici citim stringurile
    int descriptorFisier = -1;
    int descriptorMemoriePartajata = -1;
    char* pointerFisier = NULL;
    char* pointerMemoriePartajta = NULL;
    unsigned int numarOctetiMemorie = 0;
    unsigned int numarLitere = 0;
    off_t sizeFile = 0;
    char* numeFisier = NULL;
    unsigned int numberOfSections = 0;


    for(;;)
    {
        unsigned int a = 0;
        read(fd_read,&a,1);
        data=(char*)malloc(a * sizeof(char));
        read(fd_read,data,a);
        data[a]='\0';

        if(strcmp(data,"EXIT") == 0)
        {
            munmap(pointerMemoriePartajta,sizeof(char)*3019002);
            munmap(pointerFisier,sizeof(char)*sizeFile);
            pointerMemoriePartajta = NULL;
            shm_unlink("/ahRXEmY");
            close(fd_read);
            close(fd_write);
            unlink(FIFO_WRITE);
            break;
        }

        if(strcmp(data,"PING") == 0)
        {
            int nr=4;
            write(fd_write,&nr,1);
            write(fd_write,"PING",4);
            write(fd_write,&nr,1);
            write(fd_write,"PONG",4);
            unsigned int val = 98237;
            write(fd_write,&val,4);
        }
        if(strcmp(data,"CREATE_SHM") == 0)
        {
            read(fd_read,&numarOctetiMemorie,4);
            int nr2=0;

            descriptorMemoriePartajata = shm_open("/ahRXEmY",O_CREAT | O_RDWR,0664);

            if(descriptorMemoriePartajata < 0)
            {
                nr2=10;
                write(fd_write,&nr2,1);
                write(fd_write,"CREATE_SHM",10);
                nr2=5;
                write(fd_write,&nr2,1);
                write(fd_write,"ERROR",5);

            }
            ftruncate(descriptorMemoriePartajata,numarOctetiMemorie);
            pointerMemoriePartajta=(char*)mmap(0,numarOctetiMemorie,PROT_READ | PROT_WRITE,MAP_SHARED,descriptorMemoriePartajata,0);

            if(pointerMemoriePartajta == (void*)-1)
            {
                nr2=10;
                write(fd_write,&nr2,1);
                write(fd_write,"CREATE_SHM",10);
                nr2=5;
                write(fd_write,&nr2,1);
                write(fd_write,"ERROR",5);
                shm_unlink("/ahRXEmY");

            }
            nr2=10;
            write(fd_write,&nr2,1);
            write(fd_write,"CREATE_SHM",10);
            nr2=7;
            write(fd_write,&nr2,1);
            write(fd_write,"SUCCESS",7);

        }
        if(strcmp(data,"WRITE_TO_SHM") == 0)
        {

            unsigned offset = 0;
            unsigned value = 0;
            int nr = 0;

            read(fd_read,&offset,4);
            read(fd_read,&value,4);

            if(offset<0 || offset>3019002)
            {
                nr=12;
                write(fd_write,&nr,1);
                write(fd_write,"WRITE_TO_SHM",12);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);

            }
            if(offset+sizeof(value)>3019002)
            {
                nr=12;
                write(fd_write,&nr,1);
                write(fd_write,"WRITE_TO_SHM",12);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);
            }
            memcpy(&pointerMemoriePartajta[offset],(void*)&value,4);
            nr=12;
            write(fd_write,&nr,1);
            write(fd_write,"WRITE_TO_SHM",12);
            nr=7;
            write(fd_write,&nr,1);
            write(fd_write,"SUCCESS",7);
        }
        if(strcmp(data,"MAP_FILE") == 0)
        {
            int nr = 0;
            read(fd_read,&numarLitere,1);
            numeFisier=(char*)malloc(numarLitere*sizeof(char));
            read(fd_read,numeFisier,numarLitere*sizeof(char));
            numeFisier[numarLitere]='\0';
            descriptorFisier=open(numeFisier,O_RDONLY);
            if(descriptorFisier == -1)
            {
                nr=8;
                write(fd_write,&nr,1);
                write(fd_write,"MAP_FILE",8);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);

            }
            sizeFile=lseek(descriptorFisier,0,SEEK_END);
            lseek(descriptorFisier,0,SEEK_SET);

            pointerFisier=(char*)mmap(NULL,sizeFile,PROT_READ,MAP_PRIVATE,descriptorFisier,0);//aici l am mapat private
            if(pointerFisier == (void*)-1)
            {
                nr=8;
                write(fd_write,&nr,1);
                write(fd_write,"MAP_FILE",8);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);
            }
             nr=8;
            write(fd_write,&nr,1);
            write(fd_write,"MAP_FILE",8);
            nr=7;
            write(fd_write,&nr,1);
            write(fd_write,"SUCCESS",7);
            free(numeFisier);

        }
        if(strcmp(data,"READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned int offset,nrBytes;
            read(fd_read,&offset,4);
            read(fd_read,&nrBytes,4);
            int nr = 0;

            if(pointerFisier != (void*)-1)
            {
                if((offset + nrBytes) > sizeFile)
                {
                    nr=21;
                    write(fd_write,&nr,1);
                    write(fd_write,"READ_FROM_FILE_OFFSET",21);
                    nr=5;
                    write(fd_write,&nr,1);
                    write(fd_write,"ERROR",5);
                }
                else
                {
                    char* reader=(char*)malloc((nrBytes+1)*sizeof(char));
                    memcpy(reader,(pointerFisier+offset),nrBytes);
                    memcpy(pointerMemoriePartajta,reader,nrBytes);
                    nr=21;
                    write(fd_write,&nr,1);
                    write(fd_write,"READ_FROM_FILE_OFFSET",21);
                    nr=7;
                    write(fd_write,&nr,1);
                    write(fd_write,"SUCCESS",7);
                    free(reader);
                }
            }
            else
            {
                nr=21;
                write(fd_write,&nr,1);
                write(fd_write,"READ_FROM_FILE_OFFSET",21);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);
            }

        }
        if(strcmp(data,"READ_FROM_FILE_SECTION") == 0)
        {
            unsigned int numarSectiune,offset,nrBytes;
            read(fd_read,&numarSectiune,4);
            read(fd_read,&offset,4);
            read(fd_read,&nrBytes,4);
            int nr=0;

            char *magic=(char*)malloc(sizeof(char)*5);
            unsigned char version;
            unsigned int sectionOffset = 0;
            unsigned int sectionSize = 0;

            memcpy(magic,pointerFisier,4);
            magic[4]='\0';

            memcpy(&version,pointerFisier+6,1);
            memcpy(&numberOfSections,pointerFisier+7,1);
            
            if((strcmp(magic,"SFMY") == 0) && (version>=99 && version<=195) && (numberOfSections>=5 && numberOfSections<=12) && numarSectiune<=numberOfSections)
            {
                
                int pozitie=8+21*(numarSectiune-1)+13;
                memcpy(&sectionOffset,pointerFisier+pozitie,4);
                memcpy(&sectionSize,pointerFisier+pozitie+4,4);

                if(sectionSize<nrBytes+offset)
                {
                    nr=22;
                    write(fd_write,&nr,1);
                    write(fd_write,"READ_FROM_FILE_SECTION",22);
                    nr=5;
                    write(fd_write,&nr,1);
                    write(fd_write,"ERROR",5);
                }
                else
                {
                    char *reader=(char*)malloc((nrBytes+1)*sizeof(char));
                    memcpy(reader,(pointerFisier+sectionOffset+offset),nrBytes);
                    memcpy(pointerMemoriePartajta,reader,nrBytes);
                    nr=22;
                    write(fd_write,&nr,1);
                    write(fd_write,"READ_FROM_FILE_SECTION",22);
                    nr=7;
                    write(fd_write,&nr,1);
                    write(fd_write,"SUCCESS",7);
                    free(reader);
                }

            }
            else
            {
                 nr=22;
                 write(fd_write,&nr,1);
                 write(fd_write,"READ_FROM_FILE_SECTION",22);
                 nr=5;
                 write(fd_write,&nr,1);
                 write(fd_write,"ERROR",5);
            }


        }
        if(strcmp(data,"READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            int nr=0;
            unsigned int logicalOffset,nrBytes;
            read(fd_read,&logicalOffset,4);
            read(fd_read,&nrBytes,4);
            char *magic=(char*)malloc(sizeof(char)*5);
            unsigned char version;

            memcpy(magic,pointerFisier,4);
            magic[4]='\0';
            memcpy(&version,pointerFisier+6,1);
            memcpy(&numberOfSections,pointerFisier+7,1);

            if((strcmp(magic,"SFMY") == 0) && (version>=99 && version<=195) && (numberOfSections>=5 && numberOfSections<=12))
            {
                int pozitie=8;
                int offsetSectiune = 0;
                int sizeSectiune = 0;
                int aliniament = 5120;
                int sectiuniLogiceOcupate = 0;
                int ultimulOffset = 0;
                int ok=0;
               
                


                for(int i=0;i<numberOfSections;i++)
                {
                    memcpy(&offsetSectiune,pointerFisier+pozitie+13,4);
                    memcpy(&sizeSectiune,pointerFisier+pozitie+17,4);
                    if(sizeSectiune%aliniament==0)
                    {
                        sectiuniLogiceOcupate=sizeSectiune/aliniament;
                    }
                    else
                    {
                        sectiuniLogiceOcupate=sizeSectiune/aliniament+1;
                    }
                    if(logicalOffset<=ultimulOffset+(sectiuniLogiceOcupate*aliniament) && logicalOffset>=ultimulOffset)
                    {
                        char *reader=(char*)malloc(nrBytes*sizeof(char));
                         if(offsetSectiune+(logicalOffset-ultimulOffset)+nrBytes>sizeFile)
                        {
                            nr=30;
                            write(fd_write,&nr,1);
                            write(fd_write,"READ_FROM_LOGICAL_SPACE_OFFSET",30);
                            nr=5;
                            write(fd_write,&nr,1);
                            write(fd_write,"ERROR",5);
                          
                        }
                        memcpy(reader,pointerFisier+offsetSectiune+(logicalOffset-ultimulOffset),nrBytes);
                        memcpy(pointerMemoriePartajta,reader,nrBytes);
                        nr=30;
                        write(fd_write,&nr,1);
                        write(fd_write,"READ_FROM_LOGICAL_SPACE_OFFSET",30);
                        nr=7;
                        write(fd_write,&nr,1);
                        write(fd_write,"SUCCESS",7);
                        free(reader);
                        ok=1;
                        break;

                    }
                    ultimulOffset =ultimulOffset + sectiuniLogiceOcupate*aliniament;
                    pozitie=pozitie+21;

                }
                if(ok == 0)
                {
                nr=30;
                write(fd_write,&nr,1);
                write(fd_write,"READ_FROM_LOGICAL_SPACE_OFFSET",30);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);
                }
            
            }
            else
            {
                nr=30;
                write(fd_write,&nr,1);
                write(fd_write,"READ_FROM_LOGICAL_SPACE_OFFSET",30);
                nr=5;
                write(fd_write,&nr,1);
                write(fd_write,"ERROR",5);

            }

        }

        }



    return 0;
}