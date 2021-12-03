//platform agnostic file
#ifndef _IREPORTABLE_H
#define _IREPORTABLE_H

namespace general_server {
  interface_class IReportable {
  public:
    enum reportType {
      rtInformation = 0,
      rtWarning,
      rtError, //throw an exception for these instead
      rtDebug
    };

    enum reportLevel {
      rlExpected = 0,
      rlWorrying,
      rlRecoverable, //throw an expection for these instead
      rlFatal        //throw an expection for these instead
    };

    virtual const char *toString() const = 0;
  };
}

#endif

