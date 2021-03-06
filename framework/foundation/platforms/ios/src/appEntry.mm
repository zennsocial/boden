#include <bdn/init.h>
#include <bdn/ios/appEntry.h>

#import <bdn/ios/AppRunner.hh>

#include <bdn/IAppRunner.h>

#include <bdn/entry.h>

namespace bdn
{
    namespace ios
    {

        int appEntry(const std::function<P<AppControllerBase>()> &appControllerCreator, int argc, char *argv[])
        {
            int returnValue = 0;

            bdn::platformEntryWrapper(
                [&]() {
                    bdn::P<bdn::ios::AppRunner> appRunner =
                        bdn::newObj<bdn::ios::AppRunner>(appControllerCreator, argc, argv);
                    _setAppRunner(appRunner);

                    returnValue = appRunner->entry(argc, argv);
                },
                false);

            return returnValue;
        }
    }
}
