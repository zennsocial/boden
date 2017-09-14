#include <bdn/init.h>
#include <bdn/localeUtil.h>


#include <locale>


namespace bdn
{


std::locale deriveUtf8Locale(const std::locale& baseLocale)
{
    // we need to replace the default codec facets with their utf-8 counterparts
    std::locale loc = std::locale( baseLocale, new std::codecvt_utf8<wchar_t> );

    loc = std::locale( loc, new CodecVtUtf8Utf16 );
    loc = std::locale( loc, new CodecVtUtf8Utf32 );

    return loc;
}


bool isUtf8Locale(const std::locale& loc)
{
    if( std::has_facet< std::codecvt<wchar_t,char,mbstate_t> >(loc) )
    {
        const std::codecvt<wchar_t,char,mbstate_t>& codec = std::use_facet< std::codecvt<wchar_t,char,mbstate_t> >(loc);

        if( dynamic_cast< const std::codecvt_utf8<wchar_t>* >(&codec) != nullptr
            || dynamic_cast< const std::codecvt_utf8_utf16<wchar_t>* >(&codec) != nullptr)
        {
            // the codec is derived from std::codecvt_utf8 or std::codecvt_utf8_utf16.
            // So we know that it is a UTF-8 codec.
            return true;
        }
    
        // we do not recognize the codec type. But it might still be a custom codec that
        // implements UTF-8. We check for that by encoding a character sequence and
        // checking if the result is UTF-8.
        // Note that this should not cause any problems if it is the wrong codec. Since we encode,
        // rather than decode, it is unlikely that a codec bug would trigger a crash. The only
        // thing that can happen is if the specified characters cannot represented with the
        // particular multibyte codec (if it is not UTF-8). But that is a common case and
        // we can assume that all production level codecs handle that case properly.

        // Note that \u0197 produces a two byte UTF-8 sequence and \uea7d produces a three byte UTF-8 sequence.
        // We also add a pure ascii character. None of these characters fall into the surrogate pair range for
        // UTF-16, so it does not matter whether the particular codec treats wchar_t strings as UTF-16, UCS-2 or
        // UTF-32. And if the encoded data matches the UTF-8 data for all three characters
        // then we can be reasonably sure that the codec is indeed UTF-8 and that it is not a coincidence that
        // the encoded sequences are the same.
        const wchar_t* pInBegin = L"g\u0197\uea7d";
        const char*    pExpectedUtf8 = u8"g\u0197\uea7d";
        const wchar_t* pInEnd = pInBegin + wcslen(pInBegin);
        const wchar_t* pInNext = nullptr;
        
        // the UTF-8 data would be 6 bytes long.
        // But we provide a much larger buffer to ensure that the encoded data
        // can fit for all codecs. If the data does not fit then that might trigger a bug in the codec
        // that leads to a crash.
        char outBuffer[3*8+1] = {0};
        char* pOutEnd = outBuffer + sizeof(outBuffer) -1;
        char* pOutNext = outBuffer;

        std::mbstate_t state = std::mbstate_t();

        std::codecvt_base::result result = codec.out(
            state,
            pInBegin,
            pInEnd,
            pInNext,
            outBuffer,
            pOutEnd,
            pOutNext
            );

        if(result==std::codecvt_base::ok)
        {
            // the codec says that it was able to encode the data.
            // Verify that the result is what we expected.
            size_t expectedLength = strlen(pExpectedUtf8);
            
            if(pOutNext == outBuffer+expectedLength
                && memcmp(outBuffer, pExpectedUtf8, expectedLength)==0)
            {
                // the encoded data is UTF-8
                return true;
            }
        }
    }

    // not utf8
    return false;
}



}

