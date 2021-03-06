//          Copyright Lars Viklund 2008 - 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "../../SDK/foobar2000.h"
#include <deque>
#include <vector>

#include "RefPointer.h"

namespace wave
{
	template <typename T>
	struct array_sink
	{
		virtual void set(T const* data, size_t element_count) const = 0;
	};

	template <typename T>
	struct list_array_sink : array_sink<T>
	{
		pfc::list_base_t<T>& l;
		list_array_sink(pfc::list_base_t<T>& l) : l(l) {}
		virtual void set(T const* data, size_t element_count) const { l.remove_all(); l.add_items_fromptr(data, element_count); }
	};

	template <typename T>
	struct deque_array_sink : array_sink<T>
	{
		std::deque<T>& t;

		deque_array_sink(std::deque<T>& t) : t(t) {}
		virtual void set(T const* data, size_t element_count) const override
		{
			t.clear();
			t.assign(data, data + element_count);
		}
	};

	struct waveform : ref_base
	{
		virtual bool get_field(char const* what, unsigned index, array_sink<float> const& out) abstract;
		virtual unsigned get_channel_count() const abstract;
		virtual unsigned get_channel_map() const abstract;
	};
	
	ref_ptr<waveform> make_placeholder_waveform();
	ref_ptr<waveform> downmix_waveform(ref_ptr<waveform> in, size_t target_channels);
}
