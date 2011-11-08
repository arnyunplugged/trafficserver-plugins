#include "FailureInfo.h"


static int LOWER_CUT_OFF=300;
static int HIGHER_CUT_OFF=1000;

void FailureInfo::registerSuccFail(bool isSuccess)
{
    struct timeval currTime,result;
    gettimeofday(&currTime,NULL);
    timersub(&currTime,&_start,&result);

    if( ( result.tv_sec*1000000+result.tv_usec )   > (WINDOW_SIZE*1000) )
    {
        _windowMarker=++_windowMarker%_totalSlots;
      
        if(_windowMarker==_totalSlots-1)
        {
            ++_windowsPassed;
            double avg=0;
            for(size_t i=0;i<_totalSlots;i++)
            {
                if(_statistics[i].first >0)
                {
                    avg+=_statistics[i].first/(_statistics[i].first+_statistics[i].second);
                }
            }
            _avgOverWindow+=avg/_windowsPassed;
            _debugLog(_debug_tag.c_str(),"[%s] current average over window is %lf",__FUNCTION__,_avgOverWindow);
        }
    
        gettimeofday(&_start,NULL);
    }

   if(isSuccess)
   {
       _statistics[_windowMarker].second++;
   } 
   
   else
   {
       _statistics[_windowMarker].first++;
   }
}
    
bool FailureInfo::isAttemptReq()
{
    double avg=0;
    for(size_t i=0;i<_totalSlots;i++)
    {
        if(_statistics[i].first >0)
        {
            avg+=_statistics[i].first/(_statistics[i].first+_statistics[i].second);
            
        }
    }
    
    if(avg) { 
        //Average it out for time being
        avg=avg/_totalSlots;
        double prob;

        if(avg*1000<LOWER_CUT_OFF) {
            prob=avg;

        } else {
            double mapFactor=( ( (avg*1000-LOWER_CUT_OFF)*(avg*1000-LOWER_CUT_OFF) ) / (HIGHER_CUT_OFF-LOWER_CUT_OFF ) )+LOWER_CUT_OFF;
            prob=mapFactor/1000;
        }
        
        if(static_cast<int>(prob))
            prob=_avgOverWindow;
        
        _debugLog(_debug_tag.c_str(),"[%s] Calculated probability is %lf",__FUNCTION__,prob);
        int decision=rand()%100;

        if(decision<prob*100) {
            _debugLog(_debug_tag.c_str(),"[%s] fetch request will not be added for an attempt request",__FUNCTION__);
            return (_requestMade=false);
        }
    }
        
    _debugLog(_debug_tag.c_str(),"[%s] fetch request will be added for an attempt request",__FUNCTION__);
    return true;
}