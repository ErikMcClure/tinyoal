/* Example 00 - Testbed
 * -------------------------
 * This enables the wave writer backend and runs a series of tests to ensure TinyOAL
 * is working as intended.
 *
 * Copyright ©2017 Black Sphere Studios
 */

#include "tinyoal/TinyOAL.h"
#include <iostream>
#include <fstream>
#include <time.h>

#ifdef BSS_PLATFORM_WIN32
#define SLEEP(n) _sleep(n)
#else
#include <unistd.h>
#define SLEEP(n) usleep(n*1000) //translate milliseconds to microseconds
#endif

using namespace tinyoal;
using namespace bss;
  
const char* trimpath(const char* path)
{
	const char* r=strrchr(path,'/');
	const char* r2=strrchr(path,'\\');
  r=bssmax(r,r2);
  return (!r)?path:(r+1);
}

#define STREAMLOG(s,level) s << "[" << trimpath(__FILE__) << ':' << __LINE__ << "] " << level << ": "
std::ofstream _failedtests("../bin/failedtests.txt",std::ios_base::out|std::ios_base::trunc);

// Define test related stuff
struct TESTDEF
{
  typedef std::pair<size_t,size_t> RETPAIR;
  const char* NAME;
  RETPAIR (*FUNC)();
};

#define BEGINTEST TESTDEF::RETPAIR __testret(0,0)
#define ENDTEST return __testret
#define FAILEDTEST(t) STREAMLOG(_failedtests,"WARNING") << "Test #" << __testret.first << " Failed  < " << MAKESTRING(t) << " >" << std::endl
#define TEST(t) { ++__testret.first; try { if(t) ++__testret.second; else FAILEDTEST(t); } catch(...) { FAILEDTEST(t); } }
#define TESTERROR(t, e) { ++__testret.first; try { (t); FAILEDTEST(t); } catch(e) { ++__testret.second; } }
#define TESTERR(t) TESTERROR(t,...)
#define TESTNOERROR(t) { ++__testret.first; try { (t); ++__testret.second; } catch(...) { FAILEDTEST(t); } }
#define TESTCOUNT(c,t) { for(uint32_t i = 0; i < c; ++i) TEST(t) }
#define TESTCOUNTALL(c,t) { bool __val=true; for(uint32_t i = 0; i < c; ++i) __val=__val&&(t); TEST(__val); }
#define TESTFOUR(s,a,b,c,d) TEST(((s)[0]==(a)) && ((s)[1]==(b)) && ((s)[2]==(c)) && ((s)[3]==(d)))
#define TESTALLFOUR(s,a) TEST(((s)[0]==(a)) && ((s)[1]==(a)) && ((s)[2]==(a)) && ((s)[3]==(a)))
#define TESTRELFOUR(s,a,b,c,d) TEST(fcompare((s)[0],(a)) && fcompare((s)[1],(b)) && fcompare((s)[2],(c)) && fcompare((s)[3],(d)))

TESTDEF::RETPAIR test_AudioResource(const char* RES, const char* SEAMLESS, const char* strlog, double length)
{
  BEGINTEST;
  static TinyOAL engine(4); // Initialize the engine with default number of buffers

  AudioResource* resnorm = AudioResource::Create(RES, 0);
  AudioResource* rescopy = AudioResource::Create(RES, (TINYOAL_FLAG)TINYOAL_COPYINTOMEMORY);
  AudioResource* reswave = AudioResource::Create(RES, (TINYOAL_FLAG)TINYOAL_FORCETOWAVE);
  TEST(resnorm != 0);
  TEST(rescopy != 0);
  TEST(reswave != 0);

  auto fn = [&](Audio* r, AudioResource* res, TINYOAL_FLAG managed) {
    TEST(r != 0);
    TEST(r->IsPlaying());
    r->Pause();
    TEST(r->SkipSeconds(1.0));
    TEST(r->IsWhere() == 44100);
    r->Play();
    r->SetVolume(0.5f);
    TEST(r->GetVolume() == 0.5f)
      r->SetPitch(1.5f);
    TEST(r->GetPitch() == 1.5f);
    r->SetLoopPointSeconds(2.0);
    TEST(r->GetLoopPoint() == 88200);
    r->SetPosition(1, 0, 1);
    const float* pos = r->GetPosition();
    TEST(pos[0] == 1.0f && pos[1] == 0.0f && pos[2] == 1.0f);
    TEST(r->GetFlags() == (TINYOAL_ISPLAYING | managed | res->GetFlags()));
    TEST(r->GetResource() == res);
    r->Pause();
    TEST(!r->IsPlaying());
    TEST(r->Play());
    TEST(r->IsPlaying());
    TEST(r->Play());
    TEST(r->IsPlaying());
    TEST(r->Update());
    Audio cr(*r);
    //TEST(cr.IsWhere()==r->IsWhere()); // This is too time-dependent
    TEST(cr.IsPlaying());
    TEST(cr.GetResource() == res);
    Audio mv(std::move(*r));
    r->Stop();
    TEST(mv.IsPlaying());
    mv.Stop();
    if(!managed)
    {
      TEST(!r->IsPlaying());
      TEST(!r->IsWhere());
      TEST(!mv.IsPlaying());
      TEST(!mv.IsWhere());
      mv.Stop();
      TEST(mv.Play());
      mv.Stop();
    }
    TEST(res->GetLoopPoint() == -1LL);
    res->SetLoopPoint(9);
    TEST(res->GetLoopPoint() == 9);
    res->SetFlags(res->GetFlags() | TINYOAL_ISPLAYING);
    TEST(res->GetFlags()&TINYOAL_ISPLAYING);
    TEST(res->GetFreq() == 44100);
    TEST(res->GetChannels() == 2);
    TEST(res->GetBufSize() > 0);
    TEST(res->GetFormat() > 0);
    TEST(res->GetBitsPerSample() == (res->GetFileType() == AudioResource::TINYOAL_FILETYPE_WAV) ? 8 : 16);
    TEST(res->GetActiveInstances() == &cr);
    TEST(res->GetInactiveInstances() == &mv);
    mv.Play();
    TEST(res->GetNumActive() == 2);
    TEST(res->GetMaxActive() == 0);
    res->SetMaxActive(2);
    TEST(res->GetMaxActive() == 2);
    TEST(res->Play() == &cr);
    res->SetMaxActive(0);

    while(res->GetActiveInstances())
      res->GetActiveInstances()->Stop();

    Audio* hold[32]; // Simulate source exhaustion
    for(int i = 0; i < 32; ++i)
      hold[i] = res->Play();

    // Currently if source exhaustion happens TinyOAL just stops playing sounds. The default max is 256 so this shouldn't be a problem.
    for(int i = 0; i < 32; ++i) {
      hold[i]->Pause();
      hold[i]->SkipSeconds(1.0);
      TEST(hold[i]->IsWhere() == 44100);
      hold[i]->Play();
      hold[i]->Update();
      hold[i]->SetVolume(0.5f);
      TEST(hold[i]->GetVolume() == 0.5f)
        hold[i]->SetPitch(1.5f);
      TEST(hold[i]->GetPitch() == 1.5f);
      hold[i]->SetLoopPointSeconds(2.0);
      TEST(hold[i]->GetLoopPoint() == 88200);
      hold[i]->SetPosition(1, 0, 1);
      hold[i]->Stop();
    }

    if(res->GetFileType() != AudioResource::TINYOAL_FILETYPE_MP3) //skip this test for MP3 because the seamless loop messes things up
    {
      TEST(res->GetLength() == length);
    }

    TEST(res->Play()->IsPlaying());
    if(res->Play()->IsPlaying())
      res->GetActiveInstances()->Stop();
  };
  fn(resnorm->Play(), resnorm, TINYOAL_MANAGED);
  fn(rescopy->Play(), rescopy, TINYOAL_MANAGED);
  Audio inst(reswave, TINYOAL_ISPLAYING);
  fn(&inst, reswave, 0);

  resnorm->Drop();
  rescopy->Drop();
  reswave->Drop();

  if(SEAMLESS)
  {
    AudioResource* loop = AudioResource::Create(SEAMLESS, 0, 0, 0);
    AudioResource* loopcopy = AudioResource::Create(SEAMLESS, (TINYOAL_FLAG)TINYOAL_COPYINTOMEMORY, 0, 0);
    AudioResource* loopwave = AudioResource::Create(SEAMLESS, (TINYOAL_FLAG)TINYOAL_FORCETOWAVE, 0, 0);
    TEST(loop != 0);
    TEST(loopcopy != 0);
    TEST(loopwave != 0);

    auto fn2 = [&](Audio* r, AudioResource* res) {
      TEST(!res->GetLoopPoint());
      TEST(!r->GetLoopPoint());
      TEST(r->IsPlaying());

      for(int i = 0; i < 10; ++i) { // Update the engine for 1 second
        engine.Update();
        SLEEP(100);
      }
      r->Stop();
    };

    const int NUMSAMPLES = 85;
    TEST(loop->GetTotalSamples() == NUMSAMPLES);
    TEST(loopcopy->GetTotalSamples() == NUMSAMPLES);
    TEST(loopwave->GetTotalSamples() == NUMSAMPLES);
    fn2(loop->Play(), loop);
    fn2(loopcopy->Play(), loopcopy);
    fn2(loopwave->Play(), loopwave);
    loop->Drop();
    loopcopy->Drop();
    loopwave->Drop();
  }

  ENDTEST;
}

TESTDEF::RETPAIR test_AudioResourceWAV()
{
  return test_AudioResource("../../media/idea549.wav", "../../media/shape.wav", "TinyOAL_WAV.txt", 25.072131519274375);
}
TESTDEF::RETPAIR test_AudioResourceOGG()
{
  return test_AudioResource("../../media/idea803.ogg", "../../media/shape.ogg", "TinyOAL_OGG.txt", 24.0);
}
TESTDEF::RETPAIR test_AudioResourceMP3()
{
  return test_AudioResource("../../media/idea813.mp3", 0, "TinyOAL_MP3.txt", 69.172244897959189);
}
TESTDEF::RETPAIR test_AudioResourceFLAC()
{
  return test_AudioResource("../../media/idea835.flac", "../../media/shape.flac", "TinyOAL_FLAC.txt", 34.040476190476191);
}
int main()
{
  // We set up openAL to use the wave writer backend, and reduce the max sources count to 16 so we can overload it.
  TinyOAL::SetSettingsStream("drivers = wave\nsources=16\n\n[wave]\nfile=out.wav");
  //ForceWin64Crash();
  //SetWorkDirToCur();
  srand(time(NULL));

  TESTDEF tests[] = {
    { "AudioResourceWAV.h", &test_AudioResourceWAV },
    { "AudioResourceOGG.h", &test_AudioResourceOGG },
    { "AudioResourceMP3.h", &test_AudioResourceMP3 },
    { "AudioResourceFLAC.h", &test_AudioResourceFLAC },
  };
  
  const size_t NUMTESTS=sizeof(tests)/sizeof(TESTDEF);

  std::cout << "TinyOAL - OpenAL-Soft Audio Engine v" << (uint32_t)TINYOAL_VERSION.Major << '.' << (uint32_t)TINYOAL_VERSION.Minor << '.' <<
    (uint32_t)TINYOAL_VERSION.Revision << ": Unit Tests\nCopyright (c)2013 Black Sphere Studios\n" << std::endl;
  const int COLUMNS[3] = { 24, 11, 8 };
  printf("%-*s %-*s %-*s\n",COLUMNS[0],"Test Name", COLUMNS[1],"Subtests", COLUMNS[2],"Pass/Fail");

  TESTDEF::RETPAIR numpassed;
  std::vector<uint32_t> failures;
  for(uint32_t i = 0; i < NUMTESTS; ++i)
  {
    numpassed=tests[i].FUNC(); //First is total, second is succeeded
    if(numpassed.first!=numpassed.second) failures.push_back(i);

    printf("%-*s %*s %-*s\n",COLUMNS[0],tests[i].NAME, COLUMNS[1], bss::StrF("%u/%u",numpassed.second,numpassed.first).c_str(), COLUMNS[2],(numpassed.first==numpassed.second)?"PASS":"FAIL");
  }

  if(failures.empty())
    std::cout << "\nAll tests passed successfully!" << std::endl;
  else
  {
    std::cout << "\nThe following tests failed: " << std::endl;
    for (uint32_t i = 0; i < failures.size(); i++)
      std::cout << "  " << tests[failures[i]].NAME << std::endl;
    std::cout << "\nThese failures indicate either a misconfiguration on your system, or a potential bug. Please report all bugs to https://code.google.com/p/tinyoal/issues/list\n\nA detailed list of failed tests was written to failedtests.txt" << std::endl;
  }

  std::cout << "\nPress Enter to exit the program." << std::endl;
  std::cin.get();

  TinyOAL::SetSettingsStream(0); // Removes the settings file we just put in.
  return 0;
}