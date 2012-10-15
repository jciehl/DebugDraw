// jeanclaude.iehl@free.fr

#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstring>

#include "Logger.h"


namespace gk {

Log::Log( )
    :
    m_slots(),
    m_output(stdout),
    m_level(MESSAGE)
{}

Log::~Log( )
{
    if(m_output != NULL && m_output != stdout)
        fclose(m_output);
}

int Log::setOutputFile( const char *filename )
{
    if(m_output != NULL && m_output != stdout)
        fclose(m_output);
    
    m_output= fopen(filename, "wt");
    return (m_output != NULL) ? 0 : -1;
}

int Log::setOutputLevel( const unsigned int level )
{
    m_level= MESSAGE;
    if(level < MESSAGE)
        m_level= level;
    //~ if(m_level < ERROR)
        //~ m_error= ERROR;
    return 0;
}

void Log::write( const unsigned int type, const char *file, const int line, const char *function, const char *format, ... )
{
    if(m_output == NULL)
        return;
    
    if(m_level < type)
        return;
    
    std::string message;
    
    // limite la taille de l'indexation des messages
    if(m_slots.size() > 1000)
        message.append("gk::Log( ): too many messages, stop.\n");
    
    else
    {
        char tmp[1024];
        
        // construit le message indexable
        if(type != MESSAGE)
        {
            // extrait le nom de fichier, sans le chemin d'acces
            const char *separator= strrchr(file, '/');  // unix 
            if(separator == NULL)
                separator= strrchr(file, '\\'); // essaye la convention windows
            
            if(separator != 0)
                separator++;    // saute le separateur, s'il est defini
            else
                separator= file;
                
        #ifndef _MSC_VER
            snprintf(tmp, sizeof(tmp), "[%s:%d]\t", separator, line);
        #else
            _snprintf(tmp, sizeof(tmp), "[%s:%d]\t", separator, line);
        #endif
            message.append(tmp);
        }
        
        va_list args;
        va_start(args, format);
        vsnprintf(tmp, sizeof(tmp), format, args);
        va_end(args);
        message.append(tmp);
    }

#ifndef VERBOSE_DEBUG   // affiche tout les messages en mode debug
    if(type == MESSAGE 
    || m_slots.insert(message).second == true)     // n'affiche que la premiere occurence des warnings et erreurs
#endif
    {
        fprintf(m_output, "%s", message.c_str());
    }
}

}       // namespace
