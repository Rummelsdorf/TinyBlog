/*
TinyBlog without DB storing everything on HDD.
(c) 2018 Albert Rummelsdorf
*/

#include <iostream>
#include <stdlib.h>
#include <fstream>
//#include <string>
//#include <regex>
#include <experimental/filesystem>
#include <boost/filesystem.hpp>
#include <dirent.h>
//#include <cgicc/CgiDefs.h>
#include <cgicc/Cgicc.h>
//#include <cgicc/HTTPHTMLHeader.h>
//#include <cgicc/HTMLClasses.h>
#include <ctime>
#include <iomanip>
using namespace std;
const long maxtime = 2147483647;
const string dirpath = "./";
const string passwordfile = "pwd";
const string cgilocation = "tinyblog";

bool replace(string& str, const string& from, const string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
/*
void encode(string& data)
{
    string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos)
    {
        switch(data[pos])
        {
        case '&':
            buffer.append("&amp;");
            break;
        case '\"':
            buffer.append("&quot;");
            break;
        case '\'':
            buffer.append("&apos;");
            break;
        case '<':
            buffer.append("&lt;");
            break;
        case '>':
            buffer.append("&gt;");
            break;
        default:
            buffer.append(&data[pos], 1);
            break;
        }
    }
    data.swap(buffer);
    replace (data,"<br><br/>","<br>");

}
*/
void decode(std::string& data)
{
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos)
    {
        switch(data[pos])
        {
        case '\n':
            buffer.append("<br/>");
            break;
        case '\r':
            buffer.append("<br>");
            break;
        default:
            buffer.append(&data[pos], 1);
            break;
        }
    }
    data.swap(buffer);
    replace (data,"<br><br/>","<br>");
}

//struct greater
//{
//    template<class T>
//    bool operator()(T const &a, T const &b) const
//    {
 //       return a > b;
//    }
//} mygreater;


struct entry
{
    string timestamp;
    int year;
    int month;
    int day;
    vector <string> contents;
};

void cat (string filepath, bool enc)
{
    try
    {
        ifstream infile(filepath);
        string dataline;
        while (getline(infile, dataline))
        {
            istringstream iss(dataline);
            if (!enc) decode(dataline);
            cout << dataline;
        }
    }
    catch (int e)
    {
        cout << "error";
    }
}

long int hex2dec(string hexnumber)
{
    stringstream stream;
    int y;
    cin >> hexnumber;
    stream << hexnumber;
    stream >> std::hex >> y;
    return y;
}

void ensure_path_exists (string dirpath)
{
    ifstream workingdir(dirpath);
    if (!workingdir)
    {
        //  create workingdir
        boost::filesystem::path p(dirpath);
        boost::filesystem::create_directory(p);
    }
}

void overwrite (string timestamp, string data)
{
    long int targettime = maxtime-hex2dec(timestamp);
    time_t tt = targettime;
    string daypath = dirpath + to_string(1900+gmtime(&tt)->tm_year)+"/"+to_string(1+gmtime(&tt)->tm_mon)+"/"+to_string(gmtime(&tt)->tm_mday);
    ensure_path_exists (daypath);
    try
    {
        ofstream file;
        file.open (daypath + "/" + timestamp);
        file << data;
        file.close();
    }
    catch (int e)
    {
        cout << "DB Error " << e;
        exit(1);
    }
}



void printsearch (string monstr, string querystring, string linkstring, int thisyear, int thismonth)
{
    int lowestyear=2017;
    int lowestmonth=thismonth-1;
    string navstr;
    if (!(monstr.empty())) { // define limits of search
 //   cout << "we seem to have a monstr";
        string tms = monstr.substr(4, 2);
        string tys = monstr.substr(0, 4);
        thisyear = stoi(tys);
        thismonth= stoi(tms);
        lowestmonth = thismonth-1;
        int previousmonth = thismonth-1;
        int previousyear = thisyear;
        int nextyear = thisyear;
        int nextmonth = thismonth + 1;
        if (thismonth == 1)
        {previousmonth=12; previousyear--;
        }
        if (thismonth == 12) {nextmonth=1;nextyear++;
        }
        navstr = "<a href=\"?q=" + querystring + "&mon=" + to_string(previousyear) + to_string(previousmonth) + linkstring +"\">Earlier</a> - <a href=\"?q=" + querystring + "&mon=" + to_string(nextyear) + to_string(nextmonth) + linkstring+"\">Later</a>";
    }
//
    //cout << lowestmonth << " " << lowestyear << " " << thisyear<< ""<< thismonth;
    vector <entry> blogresult;
    for (int myyear = thisyear; myyear>lowestyear; myyear--)
    {
        for (int mymonth =thismonth; mymonth >lowestmonth; --mymonth)
        {
        //cout << "success" << myyear << mymonth;
            for (int myday=32; myday>0; myday--)
            {
                DIR *dir2;
                struct dirent *ent2;
                // iterate directories files
                string filepathwithdir = dirpath + to_string(myyear) + "/" + to_string(mymonth)+"/"+to_string(myday);
                //cout << filepathwithdir;
                if ((dir2 = opendir (filepathwithdir.c_str())) != NULL)
                {
             //       cout << filepathwithdir;            ////////////////////////////////////
                    vector <string> result2;
                    while ((ent2 = readdir (dir2)) != NULL)
                    {
                        result2.push_back(ent2->d_name);
                    }
                    sort( result2.begin(), result2.end());
                    for (auto it2 = result2.begin() ; it2 != result2.end(); ++it2)
                    {
                        if (!(*it2 == "..")&&(!(*it2 == ".")))
                        {
                            //             cout << "\n<li><a href=\"?ts=" << *it2 << linkstring << "\">[l]</a> ";
                            //cat (filepathwithdir+"/" + *it2,false);
                            bool containsstring=false;
                            entry tupel;
                            try
                            {
                                ifstream infile(filepathwithdir+"/" + *it2);
                                string dataline;
                                tupel.timestamp = *it2;
                                while (getline(infile, dataline))
                                {
                                    //   istringstream iss(dataline);
                                 //   cout << dataline;
                                    tupel.contents.push_back(dataline);
                                    if (!(dataline.find(querystring,-1) == std::string::npos))
                                    {
                                        containsstring=true;
                              //          cout << dataline.find(querystring);     ////////////////////////////
                                    }
                                }
                                if ((containsstring)||(querystring==" "))
                                {
                                    tupel.day=myday;
                                    tupel.month=mymonth;
                                    tupel.year=myyear;
                                    blogresult.push_back(tupel);
                                }
                            }
                            catch (int e)
                            {
                                cout << "error";
                            }
                        }
                    }
                    closedir (dir2);
                }
            }
        }
    }

// print results:
    if (blogresult.size() > 1)
    {
        time_t oldtime=11111111;
        for (entry thisentry : blogresult)
        {
            std::tm time_in = { 1, 1, 1, thisentry.day, thisentry.month-1, thisentry.year - 1900 }; // 1-based day, 0-based month, year since 1900
            time_t newtime = mktime( & time_in );
            if (!(oldtime == newtime))
            {
                if (!(oldtime == 11111111)) {cout << "</ul>";}
                cout << "\n<h3>" << std::put_time(std::gmtime(&newtime), "%A, %B %d., %Y") << "</h3><ul>\n";
                oldtime = newtime;
            }
            cout << "\n<li><a href=\"?ts=" << thisentry.timestamp << linkstring << "\">[l]</a> ";
            for (string contentline : thisentry.contents)
            {
                cout << contentline;
            }
        } // end of blogentrys
    } else
    {
        cout << "<p>No entries found.<p>";
    }
    cout << navstr;
}



int main ()
{
    cgicc::Cgicc formData;
    string linkstring = "";
    string monatsstring = "";


    cout << "Content-type:text/html\r\n\r\n";
    cout << "<html lang=\"en\"><meta charset=\"utf-8\">\n";
    //<link rel=\"stylesheet\" type=\"text/css\" href=\"http://blog.fefe.de/bild.css\">";
    cout << "<head>\n";
    cout << "<title>Idea Blogs</title>\n";
    cout << "</head>\n<body><h2><a href=\"?\" style=\"text-decoration:none;color:black\">Idea Blogs</a></h2>";
    cout << "<b>This is your sub title where you can describe what the blog is all about...</b>\n";
    cout << "<p style=\"text-align:right\">Here you can put your contact info:  <a href=\"mailto:bloguser\">E-Mail</a><p>\n";

    time_t t = std::time(nullptr);
    int thisyear=(1900+gmtime(&t)->tm_year);
    int thismonth=(1+gmtime(&t)->tm_mon);
    int thisday=gmtime(&t)->tm_mday;
    char mbstr[100];
    if (thismonth < 10)
        monatsstring = "0" + to_string(thismonth);
    else monatsstring = to_string(thismonth);


    ensure_path_exists (dirpath+to_string(thisyear));
    ensure_path_exists (dirpath+to_string(thisyear)+"/"+to_string(thismonth));

    string adminpwd;
    cgicc::form_iterator fi = formData.getElement("admin");
    if( !fi->isEmpty() && fi != (*formData).end())
    {
        string temps = (string) **fi;
        if ((temps.length() > 4) && (temps.length() < 41))
        {
            string pwdfilepath = dirpath + passwordfile;
            ifstream pwdfile(pwdfilepath);
            string line;
            getline(pwdfile, line);
//            encode(line);
            if (temps == line)
            {
                adminpwd=temps;
            }
        }
    }   //  end of getting adminpassword

    string timestamp;
    fi = formData.getElement("ts");
    if( !fi->isEmpty() && fi != (*formData).end())
    {
        string temps = (string) **fi;
        if ((temps.length() > 6) && (temps.length() < 10))
            timestamp=temps;
    } // end of getting timestamp

    if (!(adminpwd.empty()))
    {   // authenticated author
        string postdata;
        fi = formData.getElement("data");
        if( !fi->isEmpty() && fi != (*formData).end())
        {
            string temps = (string) **fi;
            if (temps.length() > 4)
                postdata=temps;
        }  //  end parsing postdata

        if ((!postdata.empty()))
        {   // insert / update entry
            if (timestamp.empty())
            { // if not explicit timestamp to edit old entry, use new time for new entry
                stringstream articleid;
                articleid << std::hex << (maxtime - t);
                timestamp = articleid.str();
            }
            overwrite (timestamp,postdata);
        }
        linkstring = "&admin="+adminpwd;  // let the links know you are authed
    }

//  if single article
    if (!timestamp.empty())
    {
        // we have a ts
      //  cout << timestamp;
        time_t tt = maxtime-hex2dec(timestamp);
        int tyear=(1900+gmtime(&tt)->tm_year);
        int tmonth=(1+gmtime(&tt)->tm_mon);
        int targetday=gmtime(&tt)->tm_mday;
        string filepath = dirpath + to_string(tyear) + "/" + to_string(tmonth) + "/" + to_string(targetday) + "/" + timestamp;
        cout << "<h3>"<< std::put_time(std::gmtime(&tt), "%A, %B %d., %Y") << "</h3><ul>" << '\n';
        cout << "<li>" << "<a href=\"?ts=" << timestamp << "\">[l]</a> ";
        if (!linkstring.empty())
        {
            cout << "<h3>Edit:</h3><form action=\""<< cgilocation <<"\" method=\"POST\"><textarea cols=\"80\" rows=\"10\" name=\"data\">";
            cat (filepath,true);
            cout << "</textarea><br/><input type=\"hidden\" value=\""<< adminpwd << "\" name=\"admin\">";
            cout << "<input type=\"hidden\" value=\""<< timestamp << "\" name=\"ts\">";
            cout << "<input type=\"submit\" value=\"Submit\"></form>";
        }
        else cat (filepath,false);
        if (tmonth < 10)
            monatsstring = "0" + to_string(tmonth);
        else monatsstring = to_string(tmonth);
        cout << "</ul><p><div style=\"text-align:center\"><a href=\"?mon="<< tyear << monatsstring << linkstring <<"\">Whole month</a></div>";
    }
    else
    {
        // no timestamp - search whole month  // get month to display
        string monstr;
        fi = formData.getElement("mon");
        if( !fi->isEmpty() && fi != (*formData).end())
        {
            string temps = (string) **fi;
            if ((temps.length() > 4)&&(temps.length() < 7))
                monstr=temps;
        }

        string querystr;
        fi = formData.getElement("q");
        if( !fi->isEmpty() && fi != (*formData).end())
        {
            string temps = (string) **fi;
            if ((temps.length() > 2)&&(temps.length() < 227))
                querystr=temps;
        }

        if (!querystr.empty()) {
            cout << "<h2> Results for " << querystr << "</h2>";
            printsearch(monstr,querystr,linkstring,thisyear,thismonth);
        } else {
            printsearch(monstr," ",linkstring,thisyear,thismonth);
        }
    }
// display footer best used before  2038
    cout << "</body></html>";
}
