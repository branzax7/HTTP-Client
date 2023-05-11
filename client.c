#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

void c_register();
char *c_login();
char *c_enter_library(char **cookies);
char *get_json_from_response(char *str);
void c_get_books(char *token, char **cookies);
void c_get_book(char *token);
void c_add_book(char *token);
void c_logout(char **cookies);

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    char *cookies[] = {};
    char *token = NULL;

    char command[100];
    while (1)
    {
        scanf("%s", command);
        if (strcmp(command, "register") == 0)
        {
            c_register();
        }

        if (strcmp(command, "login") == 0)
        {
            cookies[0] = c_login();
            printf("Cookie::>>%s\n", cookies[0]);
        }

        if (strcmp(command, "enter_library") == 0)
        {
            token = c_enter_library(cookies);
        }

        if (strcmp(command, "get_books") == 0)
        {
            c_get_books(token, cookies);
        }

        if (strcmp(command, "get_book") == 0)
        {
            c_get_book(token);
        }

        if (strcmp(command, "add_book") == 0)
        {
            c_add_book(token);
        }

        if (strcmp(command, "delete_book") == 0)
        {
        }

        if (strcmp(command, "logout") == 0)
        {
            c_logout(cookies);
            // blocheaza accesul la biblioteca care se face doar pe baza de token, necontand cookie-ul
            token = NULL;
        }

        if (strcmp(command, "exit") == 0)
        {
            return 0;
        }
    }

    return 0;
}

void c_register()
{
    char *message;
    char *response;
    char username[100];
    char password[100];
    char user[100] = "username=";
    char passwd[100] = "password=";

    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);
    strcat(user, username);
    strcat(passwd, password);

    printf("user:%s\n", user);
    printf("passs:%s\n", passwd);

    int sockfd;
    scanf("username=%s\n", username);
    scanf("password=%s\n", password);

    char *param1[] = {user, passwd};
    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/auth/register", "application/x-www-form-urlencoded", param1, 2, NULL, 0, NULL);
    puts("Mesaj:--------------------------");
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts("Response:------------------------");
    // puts(response);
    char *p = strstr(response, "OK");
    if (p != NULL)
    {
        puts("200 - OK - Utilizator înregistrat cu succes!");
    }
    else
    {
        puts("400 - Bad Request - Utilizatorul e deja luat");
    }
}

char *c_login()
{
    char *message;
    char *response;
    char username[100];
    char password[100];
    char user[100] = "username=";
    char passwd[100] = "password=";

    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);
    strcat(user, username);
    strcat(passwd, password);

    printf("user:%s\n", user);
    printf("passs:%s\n", passwd);

    int sockfd;
    scanf("username=%s\n", username);
    scanf("password=%s\n", password);

    char *param1[] = {user, passwd};
    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/auth/login", "application/x-www-form-urlencoded", param1, 2, NULL, 0, NULL);
    puts("Mesaj:--------------------------");
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts("Response:------------------------");
    puts(response);

    char *p = strstr(response, "Set-Cookie");
    if (p == NULL)
        puts("Credentiale gresite");
    printf("StrSTR: %s\n", p);
    char *loginCookie = strtok(p + strlen("Set-Cookie: "), ";");
    printf("StrTok:%s\n", loginCookie);

    return loginCookie;
}

char *c_enter_library(char **cookies)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    char *jwt = get_json_from_response(response);
    printf("+++++>>>%s\n", jwt);
    JSON_Value *root_value = json_parse_string(jwt);
    JSON_Object *root_object = json_value_get_object(root_value);
    const char *token = json_object_dotget_string(root_object, "token");
    return token;
}

// de mutat??
char *get_json_from_response(char *str)
{
    char *start = strstr(str, "{\"token\":");
    if (start != NULL)
    {
        char *end = strstr(start, "}");
        if (end != NULL)
        {
            size_t len = end - start + 1;
            char *token = strndup(start, len);
            // printf("Token: %s\n", token);
            return token;
            free(token);
        }
    }
    printf("Token not found.\n");
    return NULL;
}

void c_get_books(char *token, char **cookies)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/books", NULL, cookies, 1, token);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
}

// de adaugat si cookie-ul!!!!
void c_add_book(char *token)
{
    char *message;
    char *response;
    int sockfd;

    char title[100];
    char author[100];
    char genre[100];
    char publisher[100];
    int page_count;

    printf("title=");
    scanf("%s", title);
    printf("author=");
    scanf("%s", author);
    printf("genre=");
    scanf("%s", genre);
    printf("publisher=");
    scanf("%s", publisher);
    printf("page_count=");
    scanf("%d", &page_count);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", page_count);

    char *json_string = json_serialize_to_string_pretty(root_value);

    char *body_data[] = {json_string};

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/library/books", "application/json", body_data, 1, NULL, 0, token);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
}

void c_get_book(char *token)
{
    char *message;
    char *response;
    int sockfd;

    char id[10];
    printf("id=");
    scanf("%s", id);

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/books", id, NULL, 0, token);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
}

void c_logout(char **cookies)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
}
