#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "curl/include/curl/curl.h"
#include "curl/include/curl/easy.h"
#include "json-cpp/include/json.h"
#include "base64.h"
#include <assert.h>

#define MAX_BUFFER_SIZE 512
#define MAX_BODY_SIZE 1000000
#define _VOICE_
#define _KNOWLEDGE_
#define _TTL_

char buffer[MAX_BUFFER_SIZE];

std::string strtextvoice="";                 //
std::string strtextknowledge = "";       //


unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}


static size_t writefunc(void *ptr, size_t size, size_t nmemb, char **result)
{
    size_t result_len = size * nmemb;
    *result = (char *)realloc(*result, result_len + 1);
    if (*result == NULL)
    {
        printf("realloc failure!\n");
        return 1;
    }
    memcpy(*result, ptr, result_len);
    (*result)[result_len] = '\0';
    //printf("%s\n", *result);
    return result_len;
}

/*
*   解析图灵服务器返回的Json string
*/
int parseJsonResonse(std::string input)
{
   Json::Value root;
   Json::Reader reader;
   bool parsingSuccessful = reader.parse(input, root);
   if(!parsingSuccessful)
   {
       std::cout<<"!!! Failed to parse the response data"<< std::endl;
        return -1;
   }
   const Json::Value code = root["code"];
   const Json::Value text = root["text"];

   //result = text.asString()();

   //std::cout<< "response code:" << code << std::endl;
   //std::cout<< "response text:" << result << std::endl;

   return 0;
}
int parseJsonVoice(char* input,std::string* voice)
{
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(input, root);
    if(!parsingSuccessful)
    {
        std::cout<<"!!! Failed to parse the response data"<< std::endl;
         return -1;
    }
    *voice = root.get("result","")[0].asString();      //voice
    //printf("result=%s\r\n",*voice .c_str());
}
int parseJsonKnowledge(char* input,std::string* knowledge)
{
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(input, root);
    if(!parsingSuccessful)
    {
        std::cout<<"!!! Failed to parse the response data"<< std::endl;
         return -1;
    }
    *knowledge = root.get("tts","").asString();      //knowledge
    //printf("result=%s\r\n",*knowledge .c_str());
}
int main (int argc,char* argv[])
{

    if (argc != 2)
    {
        printf("Usage: %s audiofile\n", argv[0]);
        return -1;
    }
    printf("read audio file........\n\r");
    FILE *fp = NULL;
    fp = fopen(argv[1], "r");
    if (NULL == fp)
    {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    printf("read file ok ........\n\r");
    int content_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *audiodata = (char *)malloc(content_len);
    fread(audiodata, content_len, sizeof(char), fp);

    //put your own params here
    char *cuid = "00:0c:29:05:50:fe";
    char *apiKey = "lIpp8p5vH8RbSemWukAs5cVx";
    char *secretKey = "bx3LsPyGOMFWb6WBNzfHinxBe7qOuXK4";

    std::string token;
    char host[MAX_BUFFER_SIZE];
    char* result = (char*)malloc(MAX_BUFFER_SIZE);
    snprintf(host, sizeof(host),
            "https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=%s&client_secret=%s",
            apiKey, secretKey);
    FILE* fpp = NULL;
    char cmd[MAX_BUFFER_SIZE];
    char* curl_cmd = "curl -s ";
    char* yinhao = "\"";
    strcpy(cmd, curl_cmd);
    strcat(cmd, yinhao);
    strcat(cmd, host);
    strcat(cmd, yinhao);
    fpp = popen(cmd, "r");
    fgets(result, MAX_BUFFER_SIZE, fpp);
    pclose(fpp);
    //printf("%s\n\r",result);
    if (result != NULL)
    {
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(result, root, false))
        {
            token = root.get("access_token","").asString();
        }
    }
    printf("access token = %s\r\n",token.c_str());
    memset(host, 0, sizeof(host));
    snprintf(host, sizeof(host), "%s", "http://vop.baidu.com/server_api");

#ifdef _VOICE_
     //method 1
    char tmp[MAX_BUFFER_SIZE];
    memset(tmp, 0, sizeof(tmp));
    char body[MAX_BODY_SIZE];
    memset(body, 0, sizeof(body));
    std::string decode_data = base64_encode((const unsigned char *)audiodata, content_len);
    if (0 == decode_data.length())
    {
        printf("base64 encoded data is empty.\n");
        return 1;
    }
    else
    {
        //printf("base64 encoded success!\n\r");
    }

    Json::Value buffer;
    Json::FastWriter trans;

    buffer["format"]  = "pcm";
    buffer["rate"]    = 8000;
    buffer["channel"] = 1;
    buffer["token"]   = token.c_str();
    buffer["cuid"]    = cuid;
    buffer["speech"]  = decode_data;
    buffer["len"]     = content_len;
//    buffer["url"]  = url;
//    buffer["callback"]     = callback;

    content_len = trans.write(buffer).length();
    memcpy(body, trans.write(buffer).c_str(), content_len);

    CURL *curl;
    CURLcode res;
    char *resultBuf = NULL;
    struct curl_slist *headerlist = NULL;
    snprintf(tmp, sizeof(tmp), "%s", "Content-Type: application/json; charset=utf-8");
    headerlist = curl_slist_append(headerlist, tmp);
    snprintf(tmp, sizeof(tmp), "Content-Length: %d", content_len);
    headerlist = curl_slist_append(headerlist, tmp);

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, host);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content_len);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultBuf);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("perform curl error:%d.\n", res);
        return 1;
    }
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(curl);

    parseJsonVoice(resultBuf,&strtextvoice);
    printf("voice=%s\r\n",strtextvoice.c_str());

#endif

#ifdef _KNOWLEDGE_
    Json::Value buffertuling;
    Json::FastWriter transtuling;
    char bodytuling[MAX_BODY_SIZE];

    buffertuling["key"]  = "f8e2c683e90343b7946f0b909ba656ff";
    //buffertuling["info"]    = "百度语音提供技术支持";
    buffertuling["info"] = strtextvoice.c_str();
    buffertuling["usrid"] = "109873";

    memcpy(bodytuling, transtuling.write(buffertuling).c_str(), transtuling.write(buffertuling).length());
    //printf("%s\r\n",bodytuling);

    char hosttuling[MAX_BUFFER_SIZE];
    memset(hosttuling, 0, sizeof(hosttuling));
    snprintf(hosttuling, sizeof(hosttuling), "%s", "http://www.tuling123.com/openapi/api");
    //snprintf(hosttuling, sizeof(hosttuling), "%s", " http://47.94.75.128/openapi/api");

    CURL *curltuling;
    CURLcode restuling;
    char *resultBuftuling = NULL;

    //curl_slist *headerlisttuling;

    curl_slist *headerlisttuling = curl_slist_append(NULL,"Content-Type:application/json;charset=UTF-8");
    //headerlisttuling = curl_slist_append(headerlisttuling, "Accept:application/json");

    //http://www.tuling123.com/openapi/api?usrid=109873&key=f8e2c683e90343b7946f0b909ba656ff&info=%E5%A4%A9%E6%B0%94

    curltuling = curl_easy_init();
    curl_easy_setopt(curltuling, CURLOPT_URL, hosttuling);                       //url地址
    curl_easy_setopt(curltuling, CURLOPT_POST, 1L);                                  //设置问非0表示本次操作为post
    curl_easy_setopt(curltuling, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(curltuling, CURLOPT_HTTPHEADER, headerlisttuling);
    curl_easy_setopt(curltuling, CURLOPT_POSTFIELDS, bodytuling);                //post参数
    curl_easy_setopt(curltuling, CURLOPT_WRITEFUNCTION, writefunc);         //对返回的数据进行操作的函数地址
    curl_easy_setopt(curltuling, CURLOPT_WRITEDATA, &resultBuftuling);
    restuling = curl_easy_perform(curltuling);

    if (restuling != CURLE_OK)
    {
        printf("perform curl error:%d.\n", restuling);
        return 1;
    }
    curl_slist_free_all(headerlisttuling);
    curl_easy_cleanup(curltuling);

    parseJsonKnowledge(resultBuftuling,&strtextknowledge);
    printf("result=%s\r\n",strtextknowledge.c_str());

#endif
#ifdef _TTL_
    //TTL
    char* resultttl = (char*)malloc(MAX_BUFFER_SIZE);
    char hostttl[MAX_BUFFER_SIZE];
    memset(hostttl, 0, sizeof(hostttl));
    snprintf(hostttl, sizeof(hostttl), "%s", "http://tsn.baidu.com/text2audio");

    std::string lan ="zh";
    char ctp = 1;

    std::string strtextttl  = strtextknowledge;
    strtextttl = UrlEncode(strtextttl);

    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "?tex=%s&lan=%s&cuid=%s&ctp=%d&tok=%s", strtextttl.c_str(),lan.c_str(),cuid, ctp,token.c_str());
    strcat(hostttl, tmp);
    //printf("%s\r\n",hostttl);

    //curl_cmd = "curl -s ";
    curl_cmd = "curl -o down.wav ";
    strcpy(cmd, curl_cmd);
    strcat(cmd, yinhao);
    strcat(cmd, hostttl);
    strcat(cmd, yinhao);
    printf("%s\r\n",cmd);
    fpp = popen(cmd , "r");
    fgets(resultttl, MAX_BUFFER_SIZE, fpp);
    pclose(fpp);

#endif

    fclose(fp);
    free(audiodata);
    return 0;
}