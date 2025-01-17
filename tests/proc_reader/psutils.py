import psutil
import pwd
import time
import json
import argparse

class ProcessInfo:
    def __init__(self, pid):
        try:
            proc = psutil.Process(pid)
            self.pid = proc.pid
            self.name = proc.name()
            self.path = proc.exe()
            self.cmdline = ' '.join(proc.cmdline())
            self.work_dir = proc.cwd()
            self.ppid = proc.ppid()
            self.uid = proc.uids().real
            self.user_name = pwd.getpwuid(self.uid).pw_name
            self.gid = proc.gids().real
            self.state = proc.status()
            self.running_time = time.time() - proc.create_time()
            self.container_id = None  # Requires additional handling
            self.open_fd_count = proc.num_fds()
            self.involuntary_ctx_switches = proc.num_ctx_switches().involuntary
            self.voluntary_ctx_switches = proc.num_ctx_switches().voluntary
            self.cpu_nice = proc.nice()
            self.threads_num = proc.num_threads()
            self.env_variables = proc.environ()
            self.memory_usage = proc.memory_info().rss  # in bytes

            # IO Stats
            io_counters = proc.io_counters()
            self.iostat_read_bytes = io_counters.read_bytes
            self.iostat_write_bytes = io_counters.write_bytes
            self.iostat_read_bytes_rate = None  # Requires time sampling
            self.iostat_write_bytes_rate = None  # Requires time sampling

            # CPU usage
            cpu_times = proc.cpu_times()
            self.cpu_user_time = cpu_times.user
            self.cpu_system_time = cpu_times.system
            self.cpu_user_pct = proc.cpu_percent(interval=1)  # User CPU %
            self.cpu_system_pct = None  # Not directly available in psutil, approximation required
            self.cpu_total_pct = self.cpu_user_pct

        except psutil.NoSuchProcess:
            print(f"Process with PID {pid} does not exist.")
        except Exception as e:
            print(f"Error collecting info for PID {pid}: {e}")

    def json(self):
        return json.dumps(self.__dict__)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process Information')
    parser.add_argument('pid', type=int, help='Process ID')
    args = parser.parse_args()

    process_info = ProcessInfo(args.pid)
    print(process_info.json())
