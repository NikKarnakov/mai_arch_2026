#include <gtest/gtest.h>
#include <curl/curl.h>
#include <string>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size*nmemb);
    return size*nmemb;
}

std::string curlPost(const std::string& url, const std::string& data) {
    CURL* curl = curl_easy_init();
    std::string response;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

TEST(ApiTests, RegisterLogin) {
    std::string regResp = curlPost("http://localhost:8080/auth/register",
                                   "{\"firstName\":\"Ivan\",\"lastName\":\"Petrov\",\"login\":\"ivanp\",\"email\":\"ivan@example.com\",\"password\":\"12345\"}");
    ASSERT_NE(regResp.find("id"), std::string::npos);

    std::string loginResp = curlPost("http://localhost:8080/auth/login",
                                     "{\"login\":\"ivanp\",\"password\":\"12345\"}");
    ASSERT_NE(loginResp.find("token"), std::string::npos);
}
