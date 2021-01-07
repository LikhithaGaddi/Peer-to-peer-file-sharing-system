#include <bits/stdc++.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <regex>
#include <iostream>
#include <bits/stdc++.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cmath>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
using namespace std;
#define MAX 10024
#define SA struct sockaddr
#define CHUNK 5 * 1024

char *c_port;
char *c_add;

struct file_details
{
    int file_size;
    vector<int> bit_vector;
    int number_of_chunks;
    string path;
};

struct client_details
{
    int sockfd;
    string file_name;
    string path;
    string hash_of_file;
    vector<string> hash_of_chunks;
    int number_of_chunks;
    vector<int> bit_vector;
};

void tokenize(std::string const &str, const char *delim, std::vector<std::string> &out)
{
    char *token = strtok(const_cast<char *>(str.c_str()), delim);
    while (token != nullptr)
    {
        out.push_back(std::string(token));
        token = strtok(nullptr, delim);
    }
}

string get_file_path(string command)
{
    char buff1[FILENAME_MAX];
    string root = getcwd(buff1, FILENAME_MAX);
    string temp;
    if (command[0] == '~')
    {
        temp = string(root) + command.replace(0, 1, "");
    }
    else if (command[0] == '.' || command[0] == '/')
    {
        temp = string(root) + command.replace(0, 1, "");
    }
    else
    {
        temp = string(root) + "/" + command.replace(0, 0, "");
    }
    return temp;
}

int get_file_size(string file_name)
{
    ifstream in_file(file_name, ios::binary);
    in_file.seekg(0, ios::end);
    int file_size = in_file.tellg();
    // file_size /= 1024;
    return file_size;
}

string process_command(char buff[], map<string, file_details> &files)
{
    size_t len = strlen(buff);
    if (len == 0)
    {
        return "false";
    }
    const char *delim = " ";
    vector<string> out;
    string s = buff;
    tokenize(s, delim, out);
    if (out[0] == "create_user")
    {
        if (out.size() != 3)
        {
            cout << "Enter username and password:\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "login")
    {
        if (out.size() != 3)
        {
            cout << "Enter username and password:\n";
            return "false";
        }
        return "login";
    }
    else if (out[0] == "create_group")
    {
        if (out.size() != 2)
        {
            cout << "Enter group id:\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "join_group")
    {
        if (out.size() != 2)
        {
            cout << "Enter group id:\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "leave_group")
    {
        if (out.size() != 2)
        {
            cout << "Enter group id:\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "requests")
    {
        if (out.size() != 3)
        {
            cout << "Enter user_id and group id:\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "accept_request")
    {
        if (out.size() != 3)
        {
            cout << "Enter user_id and group id:\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "list_groups")
    {
        return "true";
    }
    else if (out[0] == "list_files")
    {
        if (out.size() != 2)
        {
            cout << "Enter both group id \n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "upload_file")
    {
        if (out.size() != 3)
        {
            cout << "Enter both group id and file name\n";
            return "false";
        }
        char ch = '/';
        size_t found;
        found = out[1].find_last_of(ch);
        if (found != string::npos)
        {
            out[1] = out[1].substr(found, out[1].length());
        }
        if (out[1][0] == '.')
        {
            out[1] = out[1].substr(1, out[1].length());
        }
        files[out[1]].file_size = get_file_size(out[1]);
        files[out[1]].number_of_chunks = ceil(files[out[1]].file_size / CHUNK);
        string path = get_file_path(out[1]);
        files[out[1]].path = path;
        vector<int> v(files[out[1]].number_of_chunks, 1);
        files[out[1]].bit_vector = v;
        return "upload_file";
    }
    else if (out[0] == "download_file")
    {
        if (out.size() != 4)
        {
            cout << "Enter both group id and file name and destination path\n";
            return "false";
        }
        return "true";
    }
    else if (out[0] == "logout")
    {
        return "true";
    }
    else if (out[0] == "show_downloads\n")
    {
    }
    else if (out[0] == "stop_share")
    {
        if (out.size() != 3)
        {
            cout << "Enter both groupid and filename:\n";
            return "false";
        }
        return "true";
    }
    else
    {
        cout << "Enter a valid command :: " << out[0] << "\n";
    }
    return "false";
}

void *socket_connection(void *p_client_socket)
{
    int si = 0;
    struct client_details *client = (struct client_details *)p_client_socket;
    string hash_of_file = client->hash_of_file;
    vector<string> hash_of_chunks = client->hash_of_chunks;
    int number_of_chunks = client->number_of_chunks;
    string path = client->path;
    int sockfd = client->sockfd;
    string file_name = client->file_name;
    const char *buff = file_name.c_str();
    if (FILE *file = fopen(path.c_str(), "r"))
    {
        fclose(file);
    }
    else
    {
        FILE *fp = fopen(path.c_str(), "w");
        // cout << "file_size" << get_file_size(file_name) << endl;
        if (fallocate(fileno(fp), 0, 0, get_file_size(file_name)) != 0)
        {
            cout << "fallocate : \n";
        }
        fclose(fp);
    }
    FILE *fin = fopen(path.c_str(), "a");
    cout << "number_of_chunks: " << number_of_chunks << endl;
    for (int i = 0; i < number_of_chunks; i++)
    {
        char buff[CHUNK];
        bzero(buff, sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        string s;
        // if (i == number_of_chunks)
        // {
        //     s = "last_send " + to_string(i) + " " + file_name;
        // }
        // else
        // {
        s = "send " + to_string(i) + " " + file_name;
        // }
        std::strcpy(buff, s.c_str());
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        cout << "before read" << endl;
        read(sockfd, buff, sizeof(buff));        
        cout << "after read" << endl;
        // cout << buff << "endl" << endl;
        char size[CHUNK];
        s = "1";
        // }
        std::strcpy(size, s.c_str());
        write(sockfd, size, sizeof(size));
        bzero(size, sizeof(size));
        memset(&size, 0, sizeof(size));
        read(sockfd, size, sizeof(size));
        // const char *delim = " ";
        // vector<string> out;
        // string s1 = buff;
        // tokenize(s1, delim, out);
        // string to_write = string(buff);
        // cout << "before seek " << i * CHUNK << " " << out[0].c_str() << " " << out[1] << endl;
        cout << buff << endl;
        cout << size << endl;
        fseek(fin, i * si, SEEK_SET);
        si = stoi(size);
        // cout << out[1].c_str() << endl;
        fwrite(buff, sizeof(char), stoi(size), fin);
    }
    fclose(fin);
    return NULL;
}

void handle_download(vector<string> out, map<string, file_details> files)
{
    vector<pair<const char *, const char *>> peer_details;
    int i;
    int number_of_clients = 0;
    string path = out[1];
    string file_name = out[2];
    for (i = 3; i < out.size() && out[i] != "file_hash"; i++)
    {
        peer_details.push_back({out[i].c_str(), out[++i].c_str()});
        number_of_clients++;
    }
    string hash_of_file = out[++i];
    vector<string> hash_of_each_chunk;
    int number_of_chunks = 0;
    for (int j = i + 1; j < out.size(); j++)
    {
        hash_of_each_chunk.push_back(out[j]);
        number_of_chunks++;
    }
    vector<int> list_of_sockfd;
    vector<vector<int>> bit_vectors(number_of_clients, vector<int>(number_of_chunks));
    for (int i = 0; i < number_of_clients; i++)
    {
        struct sockaddr_in servaddr;
        char buff[MAX];
        int sockfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            printf("socket creation failed...\n");
            exit(0);
        }
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(peer_details[i].second));
        if (inet_pton(AF_INET, peer_details[i].first, &servaddr.sin_addr) <= 0)
        {
            cout << "Error in inet_pton for " << peer_details[i].first;
            exit(-1);
        }
        if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
        {
            printf("connection with the server failed...\n");
            exit(0);
        }
        list_of_sockfd.push_back(sockfd);
        string temp = "send_bit_Vector " + file_name + " " + to_string(number_of_chunks);
        std::strcpy(buff, temp.c_str());
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        // cout << "before read";
        read(sockfd, (void *)buff, sizeof(buff));
        // cout << "after read";
        const char *delim = " ";
        vector<string> out2;
        string s1 = buff;
        tokenize(s1, delim, out2);
        if (out2[0] == "failed\n" || out2[0] == "file_not_exists")
        {
            cout << "files to get bit vector from peer server\n";
            continue;
        }
        vector<string> out1;
        string s2 = buff;
        tokenize(s2, delim, out1);
        for (int j = 0; j < out1.size(); j++)
        {
            bit_vectors[i].push_back(stoi(out1[j]));
        }
    }
    pthread_t t[number_of_clients];
    for (int i = 0; i < number_of_clients; i++)
    {
        // cout << "called"<<endl;
        struct client_details client;
        client.sockfd = list_of_sockfd[i];
        client.path = path;
        client.file_name = file_name;
        client.hash_of_file = hash_of_file;
        client.hash_of_chunks = hash_of_each_chunk;
        client.number_of_chunks = number_of_chunks;
        client.bit_vector = bit_vectors[i];
        pthread_create(&t[i], NULL, socket_connection, (void *)&client);
        pthread_join(t[i], NULL);
    }
}

char *charFromString(string str)
{

    char *path = new char[str.size() + 1];
    std::strcpy(path, str.c_str());
    return path;
}

void *func(void *p_client_socket)
{
    int *sockfd_p = (int *)p_client_socket;
    int sockfd = *sockfd_p;
    char buff[MAX];
    map<string, file_details> files;
    int n;
    for (;;)
    {
        bzero(buff, sizeof(buff));
        printf("Enter command : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        buff[n - 1] = '\0';
        string return_val = process_command(buff, files);
        if (return_val == "false")
        {
            continue;
        }
        else if (return_val == "login")
        {
            string arr2 = string(buff);
            arr2 += " " + string(c_add) + " " + string(c_port);
            std::strcpy(buff, arr2.c_str());
        }
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        const char *delim = " ";
        vector<string> out;
        string s = buff;
        tokenize(s, delim, out);
        if (out[0] == "upload_file")
        {
            handle_download(out, files);
            string copy = "download successful\n";
            std::strcpy(buff, copy.c_str());
        }
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
    close(sockfd);
    return NULL;
}

string process_server_command(char buff[])
{
    // cout << "buff : " << buff << endl;
    const char *delim = " ";
    vector<string> out;
    string s = buff;
    tokenize(s, delim, out);
    s = "failed\n";
    if (out[0] == "send_bit_Vector")
    {
        char *fi = (char *)out[1].c_str();
        FILE *f;
        f = fopen(fi, "rb");
        if (f == NULL)
        {
            s = "file_not_exists\n";
        }
        else
        {
            string x;
            for (int i = 0; i < stoi(out[2]); i++)
            {
                x += "1 ";
            }
            s = x;
        }
    }
    else if (out[0] == "send")
    {
        cout << "send" << endl;
        int chunk_number = stoi(out[1]);
        FILE *fp = fopen(out[2].c_str(), "r");
        if (fp == NULL)
        {
            s = "\nFile null";
        }
        else
        {
            int fs = fseek(fp, chunk_number * CHUNK, SEEK_SET);
            if (fs != 0)
            {
                perror("seek negative: ");
                s = "seek nonzero ";
            }
            else
            {
                char buff[CHUNK];
                int readsize = fread(buff, sizeof(char), CHUNK, fp);
                if (readsize <= 0)
                {
                    s = "not read";
                }
                else
                {
                    string str(buff);
                    cout << "readsize " << readsize << endl;
                    s = str + " " + to_string(readsize);
                    // cout << s << endl;
                }
            }
        }
    }
    // else if (out[0] == "last_send")
    // {
    //     // cout << "last_send" << endl;
    //     int chunk_number = stoi(out[1]);
    //     FILE *fp = fopen(out[2].c_str(), "r");
    //     if (fp == NULL)
    //     {
    //         s = "\nFile null";
    //     }
    //     else
    //     {
    //         int fs = fseek(fp, chunk_number * CHUNK, SEEK_SET);
    //         if (fs != 0)
    //         {
    //             perror("\n seek negative: ");
    //             s = "\nseek nonzero ";
    //         }
    //         else
    //         {
    //             char buff[CHUNK];
    //             while (!feof(fp)) // to read file
    //             {
    //                 // fucntion used to read the contents of file
    //                 fread(buff, sizeof(buff), 1, fp);
    //                 // cout << buffer;
    //             }
    //             // if (readsize <= 0)
    //             // {
    //             //     s = "\nnot read";
    //             // }
    //             // else
    //             // {
    //             string str(buff);
    //             // s = str + "\n";
    //             s = str;
    //             // }
    //         }
    //     }
    // }
    return s;
}

void *func1(void *p_client_socket)
{
    int *sockfd_p = (int *)p_client_socket;
    int sockfd = *sockfd_p;
    char buff[MAX];
    while (1)
    {
        bzero(buff, sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        string x = process_server_command(buff);
        vector<string> out;
        const char *delim = " ";
        tokenize(x, delim, out);
        // cout << out[0] << endl;
        // cout << out[1] << endl;
        cout << out[0] << endl;
        cout << x << endl;
        write(sockfd, out[0].c_str(), sizeof(out[0].c_str()));
        bzero(buff, sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        if(buff=="1")
            write(sockfd, out[1].c_str(), sizeof(out[1].c_str()));
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    const char *addr;
    const char *port;
    std::ifstream infile(argv[3]);
    string add, pt;
    getline(infile, add);
    getline(infile, pt);
    addr = add.c_str();
    port = pt.c_str();

    c_port = argv[2];
    c_add = argv[1];
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, addr, &servaddr.sin_addr) <= 0)
    {
        cout << "Error in inet_pton for " << addr;
        exit(-1);
    }
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    int *pclient = (int *)malloc(sizeof(int));
    *pclient = sockfd;
    pthread_t t;
    pthread_create(&t, NULL, func, pclient);

    int sockfd_s, connfd_s;
    socklen_t len;
    struct sockaddr_in servaddr_s, cli_s;
    sockfd_s = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_s == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr_s, sizeof(servaddr_s));
    servaddr_s.sin_family = AF_INET;
    servaddr_s.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_s.sin_port = htons(atoi(c_port));
    if ((bind(sockfd_s, (SA *)&servaddr_s, sizeof(servaddr_s))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    if ((listen(sockfd_s, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    while (true)
    {
        len = sizeof(cli_s);
        connfd_s = accept(sockfd_s, (SA *)&cli_s, &len);
        if (connfd_s < 0)
        {
            printf("server acccept failed...\n");
            exit(0);
        }

        int *pclient_s = (int *)malloc(sizeof(int));
        *pclient_s = connfd_s;
        pthread_t t_s;
        pthread_create(&t_s, NULL, func1, pclient_s);
    }

    pthread_join(t, NULL);
}
