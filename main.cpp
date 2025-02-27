#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <link.h>

namespace
{
    [[noreturn]]
    void usage()
    {
        std::fprintf(stderr, "Usage: <allocation_per_thread>\n");
        std::exit(1);
    }

    unsigned long long to_int(const char *arg_name, const char *arg)
    {
        try
        {
            return std::stoull(arg);
        }
        catch (const std::exception &e)
        {
            std::fprintf(stderr, "Invalid %s: %s\n", arg_name, arg);
            usage();
        }
    }

    struct DeleteFree
    {
        void operator()(void *ptr) const
        {
            std::free(ptr);
        }
    };

    void run_thread(const char *name, size_t allocation_chunk, unsigned long long allocation_per_thread)
    {
        std::printf("Starting: %s\n", name);
        std::vector<std::unique_ptr<void, DeleteFree>> ptrs;
        ptrs.reserve(allocation_per_thread / allocation_chunk + 1);
        for (unsigned long long allocated = 0; allocated < allocation_per_thread; allocated += allocation_chunk)
        {
            ptrs.emplace_back(std::malloc(allocation_chunk));
            if (ptrs.back() == nullptr)
            {
                std::fprintf(stderr, "Allocation failed: %s at %llu\n", name, allocated);
                return;
            }
        }
        std::printf("Done: %s\n", name);
    }

    struct CallbackData
    {
        const char *name;
        size_t allocation_chunk;
        unsigned long long allocation_per_thread;
    };

    int callback(struct dl_phdr_info *info, size_t size, void *data)
    {
        (void)info;
        (void)size;
        const auto callback_data = static_cast<const CallbackData *>(data);
        run_thread(callback_data->name, callback_data->allocation_chunk, callback_data->allocation_per_thread);
        return 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage();
    }
    constexpr size_t allocation_chunk = 30;
    const auto allocation_per_thread = to_int("allocation_per_thread", argv[1]);

    std::printf("Allocating in each thread %llu bytes = %lluMB by chunks of %luB\n", allocation_per_thread, allocation_per_thread >> 20, allocation_chunk);

    std::thread thread(run_thread, "thread", allocation_chunk, allocation_per_thread);

    CallbackData data{
        "dl_iterate_phdr",
        allocation_chunk,
        allocation_per_thread,
    };
    dl_iterate_phdr(callback, &data);

    thread.join();

    return 0;
}
