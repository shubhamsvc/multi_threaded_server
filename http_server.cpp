#include "http_server.hh"

#include <vector>

#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <ctime>


vector<string> split(const string &s, char delim)
{
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim))
  {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request)
{
//cout<<request<<"\n";
  vector<string> lines = split(request, '\n');
  // cout<<request;
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  /*
   TODO : extract the request method and URL from first_line here
  */
  this->url = first_line[1];
  this->method = split(first_line[0], ' ')[0];
  if (this->method != "GET")
  {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req)
{
  // cout<<"Handling request.....";
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;
  // cout<<"url"<<""
  response->HTTP_version = "1.0";
  // time_t curr_time = time(0);
  char buf[1000];
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z\n", &tm);
  response->time=buf;
  struct stat sb;


  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body;
    
    if (S_ISDIR(sb.st_mode))
    {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)

      */
      url = url + "/index.html";
    }

    /*
    TODO : open the file and read its contents
    */
    std::stringstream strStream;
    std::ifstream file_input;
    file_input.open(url);
    strStream << file_input.rdbuf();  
    string str = strStream.str();

    /*
    TODO : open the file and read its contents
    */
    response->body = str;
    response->content_length = to_string(str.length());
    //cout<<"\n"<<url<<"\n";
    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  else
  {
    response->status_code = "404";
    response->status_text = "Not Found";

    string str="<body> <h1>404 Page not found</h1> </body>";
    response->body = str;
    response->content_length = to_string(str.length());
    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  delete request;

  return response;
}

string HTTP_Response::get_string()
{
  /*
  TODO : implement this function
  // response->status_code = "200";
  */
 
  return "HTTP/1.1 "+this->status_code+" "+this->status_text+"\nDate: "+this->time+"Content-Type:text/html\nContent-Length: " + this->content_length + "\n\n" + this->body;

}
