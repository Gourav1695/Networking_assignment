#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <string>
#include <pthread.h>
#include <thread>
#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;
const int MAX = 26;

typedef struct varargs
{
    int sockfd;
    int packetsize;
    struct sockaddr_in *servaddr;
} varargs;
// Returns a string of random alphabets of
// length n.
string printRandomString(int n)
{
    char alphabet[MAX] = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
                          'h', 'i', 'j', 'k', 'l', 'm', 'n',
                          'o', 'p', 'q', 'r', 's', 't', 'u',
                          'v', 'w', 'x', 'y', 'z'};

    string res = "";
    for (int i = 0; i < n; i++)
        res = res + alphabet[rand() % MAX];

    return res;
}
// #define SERVER_IP "192.168.0.2"
// #define SERVER_PORT 12345
// #define NUM_TESTS 10

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit this is the mantissa
    if (f == 0.0)
        return 0; // get this special case out of the way
    // check sign and begin normalization
    if (f < 0)
    {
        sign = 1;
        fnorm = -f;
    }
    else
    {
        sign = 0;
        fnorm = f;
    } // get the normalized form of f and track the exponent
    shift = 0;
    while (fnorm >= 2.0)
    {
        fnorm /= 2.0;
        shift++;
    }
    while (fnorm < 1.0)
    {
        fnorm *= 2.0;
        shift--;
    }
    fnorm = fnorm - 1.0;                                     // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL << significandbits) + 0.5f); // get the biased exponent
    exp = shift + ((1 << (expbits - 1)) - 1);                // shift + bias
    // return the final answer
    return (sign << (bits - 1)) | (exp << (bits - expbits - 1)) | significand;
}
long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;

    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit
    if (i == 0)
        return 0.0;                                // pull the significand
    result = (i & ((1LL << significandbits) - 1)); // mask
    result /= (1LL << significandbits);            // convert back to float
    result += 1.0f;                                // add the one back on
    // deal with the exponent
    bias = (1 << (expbits - 1)) - 1;
    shift = ((i >> significandbits) & ((1LL << expbits) - 1)) - bias;
    while (shift > 0)
    {
        result *= 2.0;
        shift--;
    }
    while (shift < 0)
    {
        result /= 2.0;
        shift++;
    } // sign it
    result *= (i >> (bits - 1)) & 1 ? -1.0 : 1.0;
    return result;
}
/* ** packi16() -- store a 16-bit int into a char buffer (like htons()) */
void packi16(unsigned char *buf, unsigned int i)
{
    *buf++ = i >> 8; // this is the higher 8 bits
    *buf++ = i;      // this is the lower 8 bits
} /* ** packi32() -- store a 32-bit int into a char buffer (like htonl()) */
void packi32(unsigned char *buf, unsigned long int i)
{
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
} /* ** packi64() -- store a 64-bit int into a char buffer (like htonl()) */
void packi64(unsigned char *buf, unsigned long long int i)
{
    *buf++ = i >> 56;
    *buf++ = i >> 48;
    *buf++ = i >> 40;
    *buf++ = i >> 32;
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}
/* ** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())***/
int unpacki16(unsigned char *buf)
{
    unsigned int i2 = ((unsigned int)buf[0] << 8) | buf[1];
    int i; // change unsigned numbers to signed
    if (i2 <= 0x7fffu)
    {
        i = i2;
    }
    else
    {
        i = -1 - (unsigned int)(0xffffu - i2);
    }
    return i;
}

/* ** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like ntohs()) */
unsigned int unpacku16(unsigned char *buf)
{
    return ((unsigned int)buf[0] << 8) | buf[1];
} /* ** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl()) */
long int unpacki32(unsigned char *buf)
{
    unsigned long int i2 = ((unsigned long int)buf[0] << 24) | ((unsigned long int)buf[1] << 16) | ((unsigned long int)buf[2] << 8) | buf[3];
    long int i; // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu)
    {
        i = i2;
    }
    else
    {
        i = -1 - (long int)(0xffffffffu - i2);
    }
    return i;
}
/* ** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like ntohl()) */

unsigned long int unpacku32(unsigned char *buf)
{
    return ((unsigned long int)buf[0] << 24) | ((unsigned long int)buf[1] << 16) | ((unsigned long int)buf[2] << 8) | buf[3];
} /* ** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl()) */

long long int unpacki64(unsigned char *buf)
{
    unsigned long long int i2 = ((unsigned long long int)buf[0] << 56) | ((unsigned long long int)buf[1] << 48) | ((unsigned long long int)buf[2] << 40) | ((unsigned long long int)buf[3] << 32) | ((unsigned long long int)buf[4] << 24) | ((unsigned long long int)buf[5] << 16) | ((unsigned long long int)buf[6] << 8) | buf[7];
    long long int i; // change unsigned numbers to signed
    if (i2 <= 0x7fffffffffffffffu)
    {
        i = i2;
    }
    else
    {
        i = -1 - (long long int)(0xffffffffffffffffu - i2);
    }
    return i;
}

/* ** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like ntohl()) */
unsigned long long int unpacku64(unsigned char *buf) { return ((unsigned long long int)buf[0] << 56) | ((unsigned long long int)buf[1] << 48) | ((unsigned long long int)buf[2] << 40) | ((unsigned long long int)buf[3] << 32) | ((unsigned long long int)buf[4] << 24) | ((unsigned long long int)buf[5] << 16) | ((unsigned long long int)buf[6] << 8) | buf[7]; } /* ** pack() -- store data dictated by the format string in the buffer ** **   bits |signed   unsigned   float   string **   -----+----------------------------------**      8 |   c        C         **     16 |   h        H         f **     32 |   l        L         d **     64 |   q        Q         g **      - |                               s ** **  (16-bit unsigned length is automatically prepended to strings) */
unsigned int pack(unsigned char *buf, char *format, ...)
{
    va_list ap;
    signed char c; // 8-bit
    unsigned char C;
    int h; // 16-bit
    unsigned int H;
    long int l; // 32-bit
    unsigned long int L;
    long long int q; // 64-bit
    unsigned long long int Q;
    float f; // floats
    double d;
    long double g;
    unsigned long long int fhold;
    char *s; // strings
    unsigned int len;
    unsigned int size = 0;
    va_start(ap, format);
    for (; *format != '\0'; format++)
    {
        switch (*format)
        {
        case 'c': // 8-bit
            size += 1;
            c = (signed char)va_arg(ap, int); // promoted

            *buf++ = c;
            break;
        case 'C': // 8-bit unsigned
            size += 1;
            C = (unsigned char)va_arg(ap, unsigned int); // promoted
            *buf++ = C;
            break;
        case 'h': // 16-bit
            size += 2;
            h = va_arg(ap, int);
            packi16(buf, h);
            buf += 2;
            break;
        case 'H': // 16-bit unsigned
            size += 2;
            H = va_arg(ap, unsigned int);
            packi16(buf, H);
            buf += 2;
            break;
        case 'l': // 32-bit
            size += 4;
            l = va_arg(ap, long int);
            packi32(buf, l);
            buf += 4;
            break;
        case 'L': // 32-bit unsigned
            size += 4;
            L = va_arg(ap, unsigned long int);
            packi32(buf, L);
            buf += 4;
            break;
        case 'q': // 64-bit
            size += 8;
            q = va_arg(ap, long long int);
            packi64(buf, q);
            buf += 8;
            break;
        case 'Q': // 64-bit unsigned
            size += 8;
            Q = va_arg(ap, unsigned long long int);
            packi64(buf, Q);
            buf += 8;
            break;
        case 'f': // float-16
            size += 2;
            f = (float)va_arg(ap, double); // promoted
            fhold = pack754_32(f);         // convert to IEEE 754
            packi16(buf, fhold);
            buf += 2;
            break;
        case 'd': // float-32
            size += 4;
            d = va_arg(ap, double);
            fhold = pack754_32(d); // convert to IEEE 754
            packi32(buf, fhold);
            buf += 4;
            break;
        case 'g': // float-64
            size += 8;
            g = va_arg(ap, long double);
            fhold = pack754_64(g); // convert to IEEE 754
            packi64(buf, fhold);
            buf += 8;
            break;
        case 's': // string
            s = va_arg(ap, char *);
            len = strlen(s);
            size += len + 2;
            packi16(buf, len);
            buf += 2;
            memcpy(buf, s, len);
            buf += len;
            break;
        }
    }
    va_end(ap);
    return size;
}

/* ** unpack() -- unpack data dictated by the format string into the buffer ** **
bits |signed   unsigned   float   string **
-----+----------------------------------**
8 |   c        C         **
16 |   h        H         f **
32 |   l        L         d **
64 |   q        Q         g **      - |
s ** **  (string is extracted based on its stored length, but 's' can be **  prepended with a max length) */
void unpack(unsigned char *buf, char *format, ...)
{
    va_list ap;
    signed char *c; // 8-bit
    unsigned char *C;
    int *h; // 16-bit
    unsigned int *H;
    long int *l; // 32-bit
    unsigned long int *L;
    long long int *q; // 64-bit
    unsigned long long int *Q;
    float *f; // floats
    double *d;
    long double *g;
    unsigned long long int fhold;
    char *s;
    unsigned int len, maxstrlen = 0, count;
    va_start(ap, format);
    for (; *format != '\0'; format++)
    {
        switch (*format)
        {
        case 'c': // 8-bit
            c = va_arg(ap, signed char *);
            if (*buf <= 0x7f)
            {
                *c = *buf;
            } // re-sign
            else
            {
                *c = -1 - (unsigned char)(0xffu - *buf);
            }
            buf++;
            break;
        case 'C': // 8-bit unsigned
            C = va_arg(ap, unsigned char *);
            *C = *buf++;
            break;
        case 'h': // 16-bit
            h = va_arg(ap, int *);
            *h = unpacki16(buf);
            buf += 2;
            break;
        case 'H': // 16-bit unsigned
            H = va_arg(ap, unsigned int *);
            *H = unpacku16(buf);
            buf += 2;
            break;
        case 'l': // 32-bit
            l = va_arg(ap, long int *);
            *l = unpacki32(buf);
            buf += 4;
            break;
        case 'L': // 32-bit unsigned
            L = va_arg(ap, unsigned long int *);
            *L = unpacku32(buf);
            buf += 4;
            break;
        case 'q': // 64-bit
            q = va_arg(ap, long long int *);
            *q = unpacki64(buf);
            buf += 8;
            break;
        case 'Q': // 64-bit unsigned
            Q = va_arg(ap, unsigned long long int *);
            *Q = unpacku64(buf);
            buf += 8;
            break;
        case 'f': // float
            f = va_arg(ap, float *);
            fhold = unpacku16(buf);
            *f = unpack754_32(fhold);
            buf += 2;
            break;
        case 'd': // float-32
            d = va_arg(ap, double *);
            fhold = unpacku32(buf);
            *d = unpack754_32(fhold);
            buf += 4;
            break;
        case 'g': // float-64
            g = va_arg(ap, long double *);
            fhold = unpacku64(buf);
            *g = unpack754_64(fhold);
            buf += 8;
            break;
        case 's': // string
            s = va_arg(ap, char *);
            len = unpacku16(buf);
            buf += 2;
            if (maxstrlen > 0 && len > maxstrlen)
                count = maxstrlen - 1;
            else
                count = len;
            memcpy(s, buf, count);
            s[count] = '\0';
            buf += len;
            break;
        default:
            if (isdigit(*format))
            { // track max str len
                maxstrlen = maxstrlen * 10 + (*format - '0');
            }
        }
        if (!isdigit(*format))
            maxstrlen = 0;
    }
    va_end(ap);
}

void *receive(void *args)
{
    varargs *abc = (varargs *)args;
    int sockfd = abc->sockfd;
    int packetsize = abc->packetsize;
    struct sockaddr_in servaddr;
    unsigned char buffer[packetsize];
    int n;
    socklen_t len;
    time_t recvtime;
    printf("Sequence Number \t TTL \t RTT \n");
    while (1)
    {
        n = recvfrom(sockfd, (char *)buffer, packetsize,
                     0, (struct sockaddr *)&servaddr,
                     &len);
        recvtime = time(NULL);
        char format[] = "HLCs\0";
        char p[1024];
        int16_t i;
        int32_t starttime;
        char c;
        unpack(buffer, format, &i, &starttime, &c, &p);
        double rtt = recvtime - starttime;
        // printf("%d \t\t %c \t %f \n", i, c, rtt);
        cout << i << "\t\t" << c << "\t" << rtt << "\n";
    }
}
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    if (argc < 5)
    {
        printf("Wrong cli arguments: <serverip> <serverport> <ttl> <output file name>");
        exit(1);
    }
    char *serverip = argv[1];
    int p = 0;
    int ttl = atoi(argv[3]);
    int numpackets = 50;
    char *outfile = argv[4];
    socklen_t len;
    ssize_t n;
    clock_t start, end;
    double rtt;
    int serverport = atoi(argv[2]);
    varargs abc;
    // create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // set the server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(serverip);
    servaddr.sin_port = htons(serverport);

    // send datagrams to the server and measure RTT
    unsigned char buffer[8 + 1001];
    string s;
    int packetsize;
    // abc.sockfd = sockfd;
    // abc.packetsize = 8 + p;
    // abc.servaddr = &servaddr;
    // pthread_t tid;
    // pthread_create(&tid, NULL, receive, (void *)&abc);
    char c = ttl;
    char format[] = "HLCs\0";
    char pval[1024];
    printf("Sequence Number \t Pay \t RTT \n");
    time_t recvtime;
    fstream afile;
    afile.open(outfile, ios::out);
    // afile << "seq"
    //       << "\t"
    //       << "pay"
    //       << "\t"
    //       << "rtt"
    //       << "\n";
    int crtt = 0;
    for (int i = 0; i < numpackets; i++)
    {
        c = ttl;
        if ((i) % 5 == 0)
        {
            p += 100;
            crtt = 0;
        }
        auto offset = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();

        while ((int)c != 0)
        {
            if (c == ttl)
            {
                s = printRandomString(p);
                char format[] = "HLCs";
                packetsize = pack(buffer, format, (int16_t)(i), (int32_t)(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() - offset), (char)ttl, s.c_str());
                cout << buffer;
                buffer[packetsize + 1] = '\0';
            }
            // printf("%s", buffer);
            // cout << buffer << endl;
            sendto(sockfd, buffer, packetsize,
                   0, (const struct sockaddr *)&servaddr,
                   sizeof(servaddr));
            n = recvfrom(sockfd, (char *)buffer, packetsize,
                         0, (struct sockaddr *)&servaddr,
                         &len);
            buffer[6]--;
            c = buffer[6];
            if (c == 0)
            {
                recvtime = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() - offset;

                // int16_t i;
                int32_t starttime;
                unpack(buffer, format, &i, &starttime, &c, &pval);
                int rtt = recvtime - starttime;
                // printf("%d \t\t %c \t %f \n", i, c, rtt);
                cout << fixed << "\t" << i << " \t\t  " << p << "\t " << (int)rtt << "\n";
                crtt += rtt;
                // cout << crtt << " " << p << endl;
                if (i % 5 == 0)
                {
                    afile << ttl << "\t" << p << "\t" << crtt << "\n";
                }
            }
        }
    } // afile << "seq"
    //       << "\t"
    //       << "pay"
    //       << "\t"
    //       << "rtt"
    //       << "\n";
    afile.close();
    // pthread_join(tid, NULL);
    // close the socket
    close(sockfd);

    return 0;
}