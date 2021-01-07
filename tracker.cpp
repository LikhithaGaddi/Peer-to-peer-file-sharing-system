#include <bits/stdc++.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;
#define MAX 15000
#define SA struct sockaddr

struct User
{
    string password;
    int id;
    bool logged_in;
    string ip;
    string port;
    set<string> files;
};

struct Group
{
    int members;
    set<string> people;
    string admin;
    set<string> files;
    set<string> requests;
};

struct File
{
    int file_size;
    string file_path;
    string user_name;
    int number_of_chunks;
    string hash;
    vector<string> hash_of_chunks;
};

map<string, Group> group_details;
map<string, File> file_details;
map<string, vector<string>> file_to_userName;
static int user = 1;
map<string, User> user_details_global;

int get_file_size(string file_name)
{
    ifstream in_file(file_name, ios::binary);
    in_file.seekg(0, ios::end);
    int file_size = in_file.tellg();
    // file_size /= 1024;
    return file_size;
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

void get_hash(vector<string> &sha_of_chunk, string &hash_of_file, string file_name)
{
    char *fi = (char *)file_name.c_str();
    FILE *f;
    int i;
    f = fopen(fi, "rb");

    if (f == NULL)
    {
        perror("Error opening file\n");
        return;
    }
    char buf[5 * 1024];
    char outputBuffer[70];
    size_t sz;
    SHA_CTX md;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Init(&md);

    int blk = 0;
    string str = "";
    while (!feof(f))
    {
        if ((sz = fread((void *)buf, 1, 5 * 1024, f)) > 0)
        {
            SHA1_Init(&md);
            SHA1_Update(&md, buf, sz);
            SHA1_Final(hash, &md);
            blk++;
            for (i = 0; i < SHA_DIGEST_LENGTH; i++)
            {
                sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
            }
            sha_of_chunk.push_back(outputBuffer);
            for (i = 0; i < 20; i++)
            {
                str += outputBuffer[i];
            }
            memset(outputBuffer, 0, sizeof(outputBuffer));
            memset(hash, 0, sizeof(hash));
        }
    }
    hash_of_file = str;
    fclose(f);
}

void tokenize(std::string const &str, const char *delim, std::vector<std::string> &out)
{
    char *token = strtok(const_cast<char *>(str.c_str()), delim);
    while (token != nullptr)
    {
        out.push_back(std::string(token));
        token = strtok(nullptr, delim);
    }
}

string process_command(char buff[], string &user_logged_in, map<string, User> &user_details)
{
    const char *delim = " ";
    vector<string> out;
    string s = buff;
    tokenize(s, delim, out);
    if (out[0] == "create_user")
    {
        if (out.size() != 3)
        {
            return "Enter username and password:\n";
        }
        if (user_details_global.find(out[1]) != user_details_global.end())
        {
            return "Username exists, Try with different id\n";
        }
        user_details_global[out[1]].password = out[2];
        user_details[out[1]].password = out[2];
        user_details[out[1]].logged_in = false;
        user_details[out[1]].id = user++;
        user_details_global[out[1]].logged_in = false;
        user_details_global[out[1]].id = user_details[out[1]].id;
        return "success\n";
    }
    else if (out[0] == "login")
    {
        if (out.size() != 5)
        {
            return "Enter username and password:\n";
        }
        if (user_details.find(out[1]) == user_details.end())
        {
            return "User doesn't exists\n";
        }
        if (user_details[out[1]].password != out[2])
        {
            return "wrong password\n";
        }
        if (user_details.empty() != true && user_logged_in.size() > 0)
        {
            user_details[user_logged_in].logged_in = false;
        }
        user_logged_in = out[1];
        user_details[user_logged_in].logged_in = true;
        user_details[user_logged_in].ip = out[3];
        user_details_global[user_logged_in].logged_in = true;
        user_details_global[user_logged_in].ip = out[3];
        user_details[user_logged_in].port = out[4];
        user_details_global[user_logged_in].port = out[4];
        return "success\n";
    }
    else if (out[0] == "create_group")
    {
        if (user_logged_in == "")
        {
            return "Login to create group\n";
        }
        if (out.size() != 2)
        {
            return "Enter group id\n";
        }
        if (group_details.find(out[1]) != group_details.end())
        {
            return "Group already exists\n";
        }
        group_details[out[1]].admin = user_logged_in;
        group_details[out[1]].members = 1;
        group_details[out[1]].people.insert(user_logged_in);
        return "success\n";
    }
    else if (out[0] == "join_group")
    {
        if (user_logged_in == "")
        {
            return "Login to join group\n";
        }
        if (out.size() != 2)
        {
            return "Enter group id\n";
        }
        if (group_details.find(out[1]) == group_details.end())
        {
            return "Group doesn't exists\n";
        }
        if (group_details[out[1]].people.find(user_logged_in) != group_details[out[1]].people.end())
        {
            return "user already part of group\n";
        }
        if (user_details[user_logged_in].logged_in == false)
        {
            return "No user logged in\n";
        }
        group_details[out[1]].requests.insert(user_logged_in);
        return "success\n";
    }
    else if (out[0] == "leave_group")
    {
        if (user_logged_in == "")
        {
            return "Login to leave group\n";
        }
        if (out.size() != 2)
        {
            return "Enter group id\n";
        }
        if (group_details.find(out[1]) == group_details.end())
        {
            return "Group doesn't exits\n";
        }
        if (group_details[out[1]].people.find(user_logged_in) == group_details[out[1]].people.end())
        {
            return "Not part of group\n";
        }
        if (user_logged_in == group_details[out[1]].admin)
        {
            return "Admin can't leave the group\n";
        }
        group_details[out[1]].people.erase(user_logged_in);
        group_details[out[1]].members--;
        return "success\n";
    }
    else if (out[0] == "requests")
    {
        if (out.size() != 3)
        {
            return "Enter group id:\n";
        }
        if (group_details.find(out[2]) == group_details.end())
        {
            return "No such group\n";
        }
        string s = "\n";
        if (group_details[out[2]].requests.size() == 0)
        {
            return "No requests found\n";
        }
        for (auto it : group_details[out[2]].requests)
        {
            s += it + "\n";
        }
        return s;
    }
    else if (out[0] == "accept_request")
    {
        if (out.size() != 3)
        {
            return "Enter group id and user id\n";
        }
        if (group_details.find(out[1]) == group_details.end())
        {
            return "Group doesn't exists\n";
        }
        if (group_details[out[1]].requests.find(out[2]) == group_details[out[1]].requests.end())
        {
            return "No requests of this user to join the group\n";
        }
        if (user_logged_in != group_details[out[1]].admin)
        {
            return "Login as admin to accept requests\n";
        }
        group_details[out[1]].people.insert(out[2]);
        group_details[out[1]].requests.erase(out[2]);
        return "success\n";
    }
    else if (out[0] == "list_groups")
    {
        string g = "\n";
        if (group_details.size() == 0)
        {
            return "No groups yet";
        }
        for (auto it : group_details)
        {
            g += it.first + "\n";
        }
        return g;
    }
    else if (out[0] == "list_files")
    {
        if (group_details.find(out[1]) == group_details.end())
        {
            return "Group doesn't exists\n";
        }
        string g = "\n";
        if (group_details.size() == 0)
        {
            return "No groups yet\n";
        }
        if (group_details[out[1]].files.size() == 0)
        {
            return "No files\n";
        }
        for (auto it : group_details[out[1]].files)
        {
            g += it + "\n";
        }
        return g;
    }
    else if (out[0] == "upload_file")
    {
        if (group_details.find(out[2]) == group_details.end())
        {
            return "group doesn't exists\n";
        }
        if (user_logged_in == "")
        {
            return "login to upload a file\n";
        }
        if (group_details[out[2]].files.find(out[1]) != group_details[out[2]].files.end())
        {
            return "file already in the group\n";
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
        group_details[out[2]].files.insert(out[1]);
        file_details[out[1]].file_size = get_file_size(out[1]);
        file_details[out[1]].user_name = user_logged_in;
        file_details[out[1]].number_of_chunks = ceil(file_details[out[1]].file_size / (5 * 1024));
        file_to_userName[out[1]].push_back(user_logged_in);
        string path = get_file_path(out[1]);
        file_details[out[1]].file_path = path;
        get_hash(file_details[out[1]].hash_of_chunks, file_details[out[1]].hash, path);
        return "uploaded successfully\n";
    }
    else if (out[0] == "download_file")
    {
        // cout << out[2] << endl;
        if (group_details.find(out[1]) == group_details.end())
        {
            return "group doesn't exists\n";
        }
        if (group_details[out[1]].people.find(user_logged_in) == group_details[out[1]].people.end())
        {
            return "user not part of group\n";
        }
        if (group_details[out[1]].files.find(out[2]) == group_details[out[1]].files.end())
        {
            return "no such file in this group\n";
        }
        char ch = '/';
        size_t found;
        found = out[2].find_last_of(ch);
        if (found != string::npos)
        {
            out[2] = out[2].substr(found, out[2].length());
        }
        if (out[3][0] == '.')
        {
            out[2] = out[2].substr(1, out[2].length());
        }
        vector<string> users = file_to_userName[out[2]];
        string return_val = "upload_file " + out[3] + " " + out[2];
        for (auto user : users)
        {
            return_val += " " + user_details_global[user].ip + " " + user_details_global[user].port;
        }
        return_val += " file_hash " + file_details[out[2]].hash;
        for (auto h : file_details[out[2]].hash_of_chunks)
        {
            return_val += " " + h;
        }
        return return_val;
    }
    else if (out[0] == "logout")
    {
        user_logged_in = "";
        return "success\n";
    }
    else if (out[0] == "show_downloads\n")
    {
    }
    else if (out[0] == "stop_share")
    {
        if (group_details.find(out[1]) == group_details.end())
        {
            return "group deosn't exists\n";
        }
        if (group_details[out[1]].files.find(out[2]) == group_details[out[1]].files.end())
        {
            return "file is not being shared\n";
        }
        group_details[out[1]].files.erase(out[2]);
        return "success\n";
    }
    return "Enter a valid command\n";
}

char *charFromString(string str)
{

    char *path = new char[str.size() + 1];
    strcpy(path, str.c_str());

    return path;
}
void *func(void *p_client_socket)
{
    int sockfd = *(int *)p_client_socket;
    char buff[MAX];
    string user_logged_in;
    map<string, User> user_details;
    int n;
    for (;;)
    {
        bzero(buff, MAX);
        read(sockfd, buff, sizeof(buff));
        cout << "Fromo client: " << buff << endl;
        string x = process_command(buff, user_logged_in, user_details);
        char *buff1 = charFromString(x);
        bzero(buff, MAX);
        write(sockfd, buff1, x.size());
    }
    return NULL;
}
void *func1(void *p_client_socket)
{
    // char buff1[MAX];
    // int n = 0;
    // while ((buff1[n++] = getchar()) != '\n')
    //     ;
    // if (buff1 == "quit")
    // {
    //     exit(-1);
    // }
    string status;
    cin >> status;
    if (status == "quit")
    {
        exit(-1);
    }
    return NULL;
}
int main(int argc, char *argv[])
{
    const char *addr;
    const char *port;
    std::ifstream infile(argv[1]);
    string add, pt;
    getline(infile, add);
    getline(infile, pt);
    addr = add.c_str();
    port = pt.c_str();
    // cout << addr << " " << port << endl;
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(port));
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    pthread_t t;
    pthread_create(&t, NULL, func1, NULL);
    while (true)
    {
        len = sizeof(cli);

        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            printf("server acccept failed...\n");
            exit(0);
        }
        else
            printf("server acccept the client...\n");

        int *pclient = (int *)malloc(sizeof(int));
        *pclient = connfd;
        pthread_t t;
        pthread_create(&t, NULL, func, pclient);
    }

    close(sockfd);
}