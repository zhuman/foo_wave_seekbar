//          Copyright Lars Viklund 2008 - 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "util/Asio.h"
#include "Cache.h"
#include "waveform_sdk/Waveform.h"
#include "Job.h"
#include <atomic>
#include <list>
#include <stack>
#include <uv.h>

// {EBEABA3F-7A8E-4A54-A902-3DCF716E6A97}
extern const GUID guid_seekbar_branch;

namespace wave
{
	inline bool LocationLessThan (const playable_location_impl& x, const playable_location_impl& y)
	{
		int cmp = strcmp(x.get_path(), y.get_path());
		if (cmp == 0)
			return x.get_subsong() < y.get_subsong();
		return cmp < 0;
	}

	struct backing_store;

	bool is_of_forbidden_protocol(playable_location const& loc);

	struct cache_impl : cache
	{
		cache_impl();
		~cache_impl();

		void get_waveform(std::shared_ptr<get_request>) override;
		void remove_dead_waveforms() override;
		void compact_storage() override;
		void rescan_waveforms() override;

		void flush() override;

		bool has_waveform(playable_location const& loc) override;
		void remove_waveform(playable_location const& loc) override;

		void defer_action(std::function<void ()> fun) override;

		bool is_location_forbidden(playable_location const& loc) override;
		bool get_waveform_sync(playable_location const& loc, ref_ptr<waveform>& out) override;

		typedef std::function<void (ref_ptr<waveform>, size_t)> incremental_result_sink;

		void kick_dynamic_init();

	private:
		void open_store();
		void load_data();
		void try_delayed_init();
		void delayed_init();
		ref_ptr<waveform> process_file(playable_location_impl loc, bool user_requested, std::shared_ptr<incremental_result_sink> incremental_output = std::shared_ptr<incremental_result_sink>());

		std::atomic<bool> is_initialized;
		uv_mutex_t init_mutex;
		future_value<bool> init_sync_point;
		uv_async_t work_dispatch_work;
		uv_thread_t work_dispatch_thread;
		uv_loop_t* work_dispatch_loop;
		async_post_queue work_post_queue;

		asio::io_service io;
		std::unique_ptr<asio::io_service::work> io_work;

		uv_mutex_t important_mutex;
		std::stack<playable_location_impl> important_queue;

		pfc::string cache_filename;
		uv_mutex_t cache_mutex;
		std::list<uv_thread_t> work_threads;
		std::list<std::function<void()>> work_functions;
		typedef bool (*playable_compare_pointer)(const playable_location_impl&, const playable_location_impl&);
		abort_callback_impl flush_callback;
		std::deque<job> job_flush_queue;
		std::shared_ptr<backing_store> store;
	};

	struct cache_initquit : initquit
	{
		void on_init();
		void on_quit();
	};

	bool try_determine_song_parameters(service_ptr_t<input_decoder>& decoder, t_uint32 subsong,
		t_int64& sample_rate, t_int64& sample_count, abort_callback& abort_cb);
}
