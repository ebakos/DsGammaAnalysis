#pragma once


class AnalysisTool
{
  public:
    virtual void ProcessEvent() = 0;
    virtual void Finalize() = 0;
};