#ifdef _WIN32
#include <set>
#include <Windows.h>
#include <TlHelp32.h>
#include <gtest/gtest.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/system/win/wmi.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/time/time_utils.h>
using namespace zeus;

static const size_t kProcessSnapshotDiff = 20;

void ProcessCheck(const nlohmann::json& wmiProcess, const Process& process)
{
    auto pid = wmiProcess["ProcessId"].get<int>();
    EXPECT_EQ(process.Id(), pid);
    EXPECT_EQ(process.SessionId(), wmiProcess["SessionId"].get<int>());

    EXPECT_EQ(process.ParentId(), wmiProcess["ParentProcessId"].get<int>());
    EXPECT_EQ(process.Name(), wmiProcess["Name"].get<std::string>());
    auto exe = process.ExePathString();

    if (wmiProcess.contains("ExecutablePath"))
    {
        if (!StartWith(wmiProcess["ExecutablePath"].get<std::string>(), "\\\\"))
        {
            EXPECT_EQ(ToLowerCopy(exe), ToLowerCopy(wmiProcess["ExecutablePath"].get<std::string>()));
            EXPECT_EQ(
                ToLowerCopy(zeus::Process::GetProcessExePathString(process.Id()).value_or("")),
                ToLowerCopy(wmiProcess["ExecutablePath"].get<std::string>())
            );
        }
    }

    {
        auto dateStr = wmiProcess["CreationDate"].get<std::string>();
        auto timeStr = zeus::FormatSystemClockTime(process.CreateTime(), "%Y%m%d%H%M%S");
        EXPECT_EQ(dateStr.substr(0, 14), timeStr);
    }
}

TEST(Process, ListId)
{
    std::set<Process::PID> expectList;
    HANDLE                 hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hsnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 stcProcessInfo;
        stcProcessInfo.dwSize = sizeof(stcProcessInfo);

        for (BOOL bRet = Process32First(hsnapshot, &stcProcessInfo); bRet; bRet = Process32Next(hsnapshot, &stcProcessInfo))
        {
            expectList.emplace(stcProcessInfo.th32ProcessID);
        }
    }
    auto processes = Process::ListId();
    auto diff      = static_cast<int>(expectList.size()) - static_cast<int>(processes.size());
    ASSERT_LT(std::abs(diff), kProcessSnapshotDiff);
    bool res = true;
    for (auto ele : expectList)
    {
        if (processes.end() == processes.find(ele))
        {
            res = false;
            break;
        }
    }
    EXPECT_EQ(true, res);
}

TEST(Process, List)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto expectProcesses = query->Query("SELECT * FROM win32_Process").value();

    auto processes = Process::ListAll();
    EXPECT_LE(std::abs((int) (expectProcesses.size() - processes.size())), kProcessSnapshotDiff);
    int error = 0;
    for (auto& process : expectProcesses)
    {
        auto pid  = process["ProcessId"].get<size_t>();
        auto iter = std::find_if(
            processes.begin(), processes.end(),
            [id = pid](const Process& process)
            {
                if (process.Id() == id)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        );
        if (iter != processes.end())
        {
            if (0 == iter->Id())
            {
                continue;
            }
            ProcessCheck(process, *iter);
        }
        else
        {
            error++;
        }
    }

    EXPECT_LE(error, kProcessSnapshotDiff);
}

TEST(Process, FindPid)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto expectProcesses = query->Query("SELECT * FROM win32_Process").value();
    int  findCount       = 0;

    for (auto& process : expectProcesses)
    {
        auto pid = process["ProcessId"].get<int>();

        auto target = Process::FindProcessById(pid);
        if (target)
        {
            findCount++;

            if (0 == pid)
            {
                continue;
            }
            ProcessCheck(process, *target);
        }
    }
    EXPECT_LE(std::abs((int) (findCount - expectProcesses.size())), kProcessSnapshotDiff);
}

TEST(Process, FindName)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto expectProcesses = query->Query("SELECT * FROM win32_Process").value();
    int  findCount       = 0;

    for (auto& process : expectProcesses)
    {
        auto pid     = process["ProcessId"].get<size_t>();
        auto name    = process["Name"].get<std::string>();
        auto targets = Process::FindProcessByName(name);
        if (targets.size())
        {
            for (auto& target : targets)
            {
                if (target.Id() == pid)
                {
                    findCount++;

                    if (0 == pid)
                    {
                        continue;
                    }
                    ProcessCheck(process, target);
                    break;
                }
            }
        }
    }
    EXPECT_LE(std::abs((int) (findCount - expectProcesses.size())), kProcessSnapshotDiff);
}

TEST(Process, Find)
{
    auto targets = Process::FindProcessByName("explorer.exe");
    EXPECT_GT(targets.size(), 1);
}

TEST(Process, GetCurrentId)
{
    EXPECT_EQ(GetCurrentProcessId(), Process::GetCurrentId());
}

TEST(Process, GetCurrent)
{
    auto process = Process::GetCurrentProcess();
    EXPECT_EQ(GetCurrentProcessId(), process.Id());
    EXPECT_EQ(CurrentExe::GetProcessSessionId(), process.SessionId());
}

#endif //
