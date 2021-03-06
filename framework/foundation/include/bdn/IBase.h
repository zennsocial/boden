#ifndef BDN_IBase_H_
#define BDN_IBase_H_

#ifdef _MSC_VER
// disable warning "CLASS : inherits FUNCTION via dominance"
#pragma warning(disable : 4250)
#endif

namespace bdn
{

    /** Base interface for most classes.

        It defines the methods for reference counting.

        Classes must implement IBase to be compatible with the smart pointer
       bdn::P. Usually classes simply inherit from Base (which provides a
       default implementation), rather than implementing IBase manually.
    */
    class IBase
    {
      public:
        virtual ~IBase() {}

        /** Increases the object's reference count by one. This should normally
           NOT be called directly. Instead you should use #P smart pointers,
           which take care of this automatically.

            addRef implementations MUST be thread-safe.
            */
        virtual void addRef() const = 0;

        /** Decreases the object's reference count by one. When it reaches 0
           then the object will delete itself.

            releaseRef implementations MUST be thread-safe.
            */
        virtual void releaseRef() const = 0;
    };
}

#endif
