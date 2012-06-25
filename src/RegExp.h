#ifndef RegExp_h
#define RegExp_h

#include <regex.h>
#include <ByteArray.h>

class RegExp
{
public:
    RegExp(const ByteArray &pattern, uint32_t flags = 0)
        : mPattern(pattern), mFlags(flags), mState(Unset)
    {}

    ~RegExp()
    {
        if (mState == Success) {
            regfree(&mRegex);
        }
    }

    bool isValid() const
    {
        if (mState == Unset) {
            if (mPattern.isEmpty()) {
                mState = Error;
            } else {
                mState = regcomp(&mRegex, mPattern.constData(), mFlags) ? Error : Success;
            }
        }
        return mState == Success;
    }

    bool isEmpty() const
    {
        printf("%d %d\n", mPattern.isEmpty(), isValid());
        return mPattern.isEmpty() || !isValid();
    }

    ByteArray pattern() const
    {
        return mPattern;
    }

    struct Capture {
        Capture()
            : index(-1), size(0), byteArray(0)
        {}
        int index, size;
        const ByteArray *byteArray;

        ByteArray capture() const
        {
            if (byteArray && index >= 0 && size > 0) {
                return byteArray->mid(index, size);
            }
            return ByteArray();
        }
    };

    int indexIn(const ByteArray &string, int offset = 0, List<Capture> *caps = 0, uint32_t flags = 0) const
    {
        if (!isValid())
            return -1;
        regmatch_t captures[10];
        if (regexec(&mRegex, string.constData() + offset, sizeof(captures) / sizeof(regmatch_t), captures, flags)) {
            return -1;
        }
        if (caps) {
            for (unsigned i=0; i<sizeof(captures) / sizeof(regmatch_t); ++i) {
                if (captures[i].rm_so != -1) {
                    Capture capture;
                    capture.byteArray = &string;
                    capture.index = captures[i].rm_so;
                    capture.size = captures[i].rm_eo - capture.index;
                    caps->append(capture);
                } else {
                    break;
                }
            }
        }
        return captures[0].rm_so;
    }
private:
    const ByteArray mPattern;
    const uint32_t mFlags;
    mutable regex_t mRegex;
    enum State {
        Unset,
        Error,
        Success
    } mutable mState;
};

#endif
