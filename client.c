#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <errno.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

void c_register();
char *c_login();
char *c_enter_library(char **cookies);
void c_get_books(char *token);
void c_get_book(char *token);
void c_add_book(char *token);
void c_logout(char **cookies);
void c_delete_book(char *token);
char *get_json_from_response(char *str);
char *get_array_from_response(char *str);
int is_numeric(char *str);
int contains_digits(char *str);

int main(int argc, char *argv[])
{
    char *cookies[] = {};
    char *token = NULL;

    char command[100];
    while (1)
    {
        fgets(command, 100, stdin);
        command[strcspn(command, "\n")] = '\0';
        if (strcmp(command, "register") == 0)
        {
            c_register();
        }

        else if (strcmp(command, "login") == 0)
        {
            cookies[0] = c_login();
            // printf("Cookie::>>%s\n", cookies[0]);
            token = NULL;
        }

        else if (strcmp(command, "enter_library") == 0)
        {
            token = c_enter_library(cookies);
        }

        else if (strcmp(command, "get_books") == 0)
        {
            c_get_books(token);
        }

        else if (strcmp(command, "get_book") == 0)
        {
            c_get_book(token);
        }

        else if (strcmp(command, "add_book") == 0)
        {
            c_add_book(token);
        }

        else if (strcmp(command, "delete_book") == 0)
        {
            c_delete_book(token);
        }

        else if (strcmp(command, "logout") == 0)
        {
            c_logout(cookies);
            // blocheaza accesul la biblioteca care se face doar pe baza de token, necontand cookie-ul, prin stergerea lui
            token = NULL;
        }

        else if (strcmp(command, "exit") == 0)
        {
            return 0;
        }
        else if (strcmp(command, "") == 0)
        {
            // nimic
        }
        else
        {
            puts("Comanda gresita!!!");
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
    fgets(username, 100, stdin);
    username[strcspn(username, "\n")] = '\0';
    if (strchr(username, ' ') != NULL)
    {
        puts("Eroare! Username-ul nu trebuie sa contina spatii!");
        return;
    }
    printf("password=");
    fgets(password, 100, stdin);
    password[strcspn(password, "\n")] = '\0';
    strcat(user, username);
    strcat(passwd, password);

    int sockfd;

    char *param1[] = {user, passwd};
    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/auth/register", "application/x-www-form-urlencoded", param1, 2, NULL, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 201") != NULL)
    {
        puts("201-Created-Contul a fost inregistrat!");
    }
    if (strstr(response, "HTTP/1.1 400") != NULL)
    {
        puts("400-Bad Request-Username deja luat!");
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
    fgets(username, 100, stdin);
    username[strcspn(username, "\n")] = '\0';
    printf("password=");
    fgets(password, 100, stdin);
    password[strcspn(password, "\n")] = '\0';
    strcat(user, username);
    strcat(passwd, password);

    int sockfd;

    char *param1[] = {user, passwd};
    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/auth/login", "application/x-www-form-urlencoded", param1, 2, NULL, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK-Bun venit!");
        char *p = strstr(response, "Set-Cookie");
        char *loginCookie = strtok(p + strlen("Set-Cookie: "), ";");
        return loginCookie;
    }
    if (strstr(response, "HTTP/1.1 400") != NULL)
    {
        puts("400-Bad Request-Credentiale gresite!");
    }

    return NULL;
}

char *c_enter_library(char **cookies)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    char *jwt = get_json_from_response(response);
    JSON_Value *root_value = json_parse_string(jwt);
    JSON_Object *root_object = json_value_get_object(root_value);
    char *token = json_object_dotget_string(root_object, "token");
    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK-Accesare biblioteca cu succes!");
    }
    if (strstr(response, "HTTP/1.1 401") != NULL)
    {
        puts("401-Unauthorized-Nu sunteti logat!");
    }
    return token;
}

// de mutat??
char *get_json_from_response(char *str)
{
    char *start = strstr(str, "{\"");
    if (start != NULL)
    {
        char *end = strstr(start, "}");
        if (end != NULL)
        {
            int len = end - start + 1;
            char *token = strndup(start, len);
            return token;
            free(token);
        }
    }
    return NULL;
}

char *get_array_from_response(char *str)
{
    char *start = strstr(str, "[");
    if (start != NULL)
    {
        char *end = strstr(start, "]");
        if (end != NULL)
        {
            int len = end - start + 1;
            char *token = strndup(start, len);
            return token;
            free(token);
        }
    }
    return NULL;
}

void c_get_books(char *token)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/books", NULL, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK");
        char *carti = get_array_from_response(response);
        JSON_Value *root_value = json_parse_string(carti);
        JSON_Array *array = json_value_get_array(root_value);
        char *pretty_array = json_serialize_to_string_pretty(root_value);
        printf("%s\n", pretty_array);
    }

    if (strstr(response, "HTTP/1.1 403") != NULL)
    {
        puts("403-Forbidden-Nu aveti acces la biblioteca!");
    }
}

void c_add_book(char *token)
{
    char *message;
    char *response;
    int sockfd;

    char title[100];
    char author[100];
    char genre[100];
    char publisher[100];
    char page_count[10];

    printf("title=");
    fgets(title, 100, stdin);
    title[strcspn(title, "\n")] = '\0';
    printf("author=");
    fgets(author, 100, stdin);
    author[strcspn(author, "\n")] = '\0';
    if (contains_digits(author))
    {
        puts("Eroare! Nu poti avea cifre in numele autorului");
        return;
    }
    printf("genre=");
    fgets(genre, 100, stdin);
    genre[strcspn(genre, "\n")] = '\0';
    if (contains_digits(genre))
    {
        puts("Eroare! Nu poti avea cifre in genul cartii");
        return;
    }
    printf("publisher=");
    fgets(publisher, 100, stdin);
    publisher[strcspn(publisher, "\n")] = '\0';
    if (contains_digits(publisher))
    {
        puts("Eroare! Nu poti avea cifre in numele producatorului");
        return;
    }
    printf("page_count=");
    fgets(page_count, 10, stdin);
    page_count[strcspn(page_count, "\n")] = '\0';
    if (is_numeric(page_count) == 0)
    {
        puts("Eroare: Numarul de pagini trebuie sa reprezinte un numar");
        return;
    }
    if (atoi(page_count) < 0)
    {
        puts("Eroare: Numarul de pagini nu poate fi negativ!");
        return;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", atoi(page_count));

    char *json_string = json_serialize_to_string_pretty(root_value);
    printf("ADAUG:\n%s\n", json_string);

    char *body_data[] = {json_string};

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/library/books", "application/json", body_data, 1, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK-Carte adaugata cu succes!");
    }
    if (strstr(response, "HTTP/1.1 403") != NULL)
    {
        puts("403-Forbidden-Nu aveti acces la biblioteca!");
    }
}

void c_get_book(char *token)
{
    char *message;
    char *response;
    int sockfd;

    char id[10];
    printf("id=");
    fgets(id, 10, stdin);
    id[strcspn(id, "\n")] = '\0';
    if (is_numeric(id) == 0)
    {
        puts("Eroare: Id-ul trebuie sa reprezinte un numar");
        return;
    }

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/books", id, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK-Carte gasita cu succes!");
        char *carte = get_json_from_response(response);
        JSON_Value *root_value = json_parse_string(carte);
        char *pretty_json = json_serialize_to_string_pretty(root_value);
        printf("%s\n", pretty_json);
    }

    if (strstr(response, "HTTP/1.1 404") != NULL)
    {
        puts("404-Not Found-Nu exista carte cu acest id!");
    }

    if (strstr(response, "HTTP/1.1 403") != NULL)
    {
        puts("403-Forbidden-Nu aveti acces la biblioteca!");
    }
}

void c_logout(char **cookies)
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK-Deconectare efectuata cu succes!");
    }
    if (strstr(response, "HTTP/1.1 400") != NULL)
    {
        puts("400-Bad Request-Nu esti logat!");
    }
}

void c_delete_book(char *token)
{
    char *message;
    char *response;
    int sockfd;

    char id[10];
    printf("id=");
    fgets(id, 10, stdin);
    id[strcspn(id, "\n")] = '\0';

    if (is_numeric(id) == 0)
    {
        puts("Eroare: Id-ul trebuie sa reprezinte un numar!");
        return;
    }

    if (atoi(id) < 0)
    {
        puts("Eroare: Id-ul nu poate fi un numar negativ!");
        return;
    }

    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_delete_request("34.254.242.81:8080", "/api/v1/tema/library/books", id, NULL, 1, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    if (strstr(response, "HTTP/1.1 200") != NULL)
    {
        puts("200-OK-Carte stearsa cu succes!");
    }

    if (strstr(response, "HTTP/1.1 404") != NULL)
    {
        puts("404-Not Found-Nu exista carte cu acest id!");
    }

    if (strstr(response, "HTTP/1.1 403") != NULL)
    {
        puts("403-Forbidden-Nu aveti acces la biblioteca!");
    }
}

int is_numeric(char *str)
{
    char *endptr;
    errno = 0;

    strtol(str, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || str == endptr)
    {
        return 0;
    }

    return 1;
}

int contains_digits(char *str)
{
    int i;
    for (i = 0; i <= 9; i++)
    {
        char c = i + '0';
        if (strchr(str, c) != NULL)
        {
            return 1;
        }
    }
    return 0;
}