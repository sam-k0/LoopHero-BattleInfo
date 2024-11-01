#pragma once
#include <fstream>
#include <iterator>
#include <string>
#include <Windows.h>
#include <vector>
#include <direct.h>
#include <ostream>
#include <sstream>
#include <iomanip>
// YYTK 
#define YYSDK_PLUGIN // Declare the following code as a plugin
//#define DEBUG // enables more prints
#include "SDK/SDK.hpp"
#include "Filesystem.h"

HINSTANCE DllHandle; // Self modhandle

std::string gPluginName = "sam-k0.BattleInfo.yytk";


DllExport std::string GetPluginName() // For yytk
{
    return gPluginName;
}


namespace Misc {
    void Print(std::string s, Color c = CLR_DEFAULT)
    {
        PrintMessage(c, (gPluginName + ": " + s).c_str());
    }

    void PrintDbg(std::string s, std::string func, int line, Color c)
    {
        PrintMessage(c, (gPluginName + ": " + s + " (" + func + ":" + std::to_string(line) + ")").c_str());
    }

    bool VectorContains(std::string find, std::vector<std::string>* v)
    {
        if (std::find(v->begin(), v->end(), find) != v->end())
        {
            return true;
        }
        return false;
    }

    bool AddToVectorNoDuplicates(std::string str, std::vector<std::string>* v)
    {
        if (VectorContains(str, v))
        {
            return false;
        }
        v->push_back(str);
        return true;
    }

    inline bool FileExists(const std::string& name) {
        if (FILE* file = fopen(name.c_str(), "r")) {
            fclose(file);
            return true;
        }
        else {
            return false;
        }
    }

    // checks if a string s contains a substring subs
    bool StringHasSubstr(std::string s, std::string subs)
    {
        if (s.find(subs) != std::string::npos) {
            return true;
        }
        return false;
    }

    // checks if the vector contains a string containing a substringand returns the whole string if found
    std::string VectorFindSubstring(std::vector<std::string> strings, std::string subs)
    {
        if (strings.size() == 0)
        {
            return "";
        }

        for (std::string s : strings)
        {
            if (StringHasSubstr(s, subs))
            {
                return s;
            }
        }
        return "";
    }

    //https://www.techiedelight.com/check-if-a-string-ends-with-another-string-in-cpp/
    bool StringEndsWith(std::string const& str, std::string const& suffix) {
        if (str.length() < suffix.length()) {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    std::string GetCurrentDir() // Returns EXE directory
    {
        char cCurrentPath[FILENAME_MAX]; // get working directory into buffer
        if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
            exit(-1);
        cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; // not really required

        char* s = cCurrentPath; // save path from buffer into currentpath chararr
        std::string str(s);
        //free(s);
        return str;
    }

    std::string Join(std::vector<std::string> strings)
    {
        if (strings.size() == 0)
        {
            return "";
        }

        const char* const delim = " ";
        std::ostringstream imploded;
        std::string ret;
        std::copy(strings.begin(), strings.end(), std::ostream_iterator<std::string>(imploded, delim));
        ret = imploded.str();
        ret.pop_back();

        return ret;
    }

    void PrintArray(YYRValue var, Color c = Color::CLR_DEFAULT)
    {
        YYRValue len;
        YYRValue item;
        CallBuiltin(len, "array_length_1d", nullptr, nullptr, { var });

        for (int i = 0; i < (int)len; i++)
        {
            CallBuiltin(item, "array_get", nullptr, nullptr, { var, (double)i });
            Misc::Print(static_cast<const char*>(item), c);
        }
    }

    std::string to_string_trimmed(double value, int prec) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(prec) << value;  // Set the precision to 2 or whatever you need
        std::string str = out.str();

        // Remove trailing zeros
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);

        // Remove the decimal point if it's the last character (e.g., "5.")
        if (str.back() == '.') {
            str.pop_back();
        }

        return str;
    }
}

namespace Binds {
    void GetInstanceVariables(YYRValue& arr, YYRValue inst)
    {
        CallBuiltin(arr, "variable_instance_get_names", nullptr, nullptr, { inst });
    }

    YYRValue CallBuiltin(const std::string name, CInstance* self, CInstance* other, const std::vector<YYRValue> args)
    {
        YYRValue var;
        CallBuiltin(var, name, self, other, args);
        return var;
    }

    YYRValue CallBuiltinA(const std::string name, const std::vector<YYRValue> args)
    {
        YYRValue var;
        CallBuiltin(var, name, nullptr, nullptr, args);
        return var;
    }

    void PrintArrayInstanceVariables(YYRValue var, YYRValue inst, Color c = Color::CLR_DEFAULT)
    {
        YYRValue len;
        YYRValue item;
        YYRValue content;
        YYRValue type;
        CallBuiltin(len, "array_length_1d", nullptr, nullptr, { var });

        for (int i = 0; i < (int)len; i++)
        {
            CallBuiltin(item, "array_get", nullptr, nullptr, { var, (double)i });
            CallBuiltin(content, "variable_instance_get", nullptr, nullptr, { inst, static_cast<const char*>(item) });
            CallBuiltin(type, "typeof", nullptr, nullptr, { content });
            std::string typestr = std::string(static_cast<const char*>(type));
            std::string message = std::string(static_cast<const char*>(item)) + " -> " + std::string(static_cast<const char*>(type));

            if (typestr == "number")
            {
                message += " : " + std::to_string((int)content);
            }
            else if (typestr == "bool")
            {
                message += " : " + std::to_string(int((bool)content));
            }
            else if (typestr == "string")
            {
                message += " : " + std::string(static_cast<const char*>(content));
			}
			else
			{
                message += " : ";
			}

            Misc::Print(message);
        }
    }

}