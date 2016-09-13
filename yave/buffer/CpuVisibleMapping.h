/*******************************
Copyright (C) 2013-2016 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/
#ifndef YAVE_BUFFER_CPUVISIBLEMAPPING_H
#define YAVE_BUFFER_CPUVISIBLEMAPPING_H

#include "BufferMemoryReference.h"

namespace yave {

class CpuVisibleMapping : NonCopyable {

	public:
		template<BufferUsage Usage, BufferTransfer Transfer>
		CpuVisibleMapping(GenericBuffer<Usage, MemoryFlags::CpuVisible, Transfer>& buffer) : CpuVisibleMapping(&buffer) {
		}

		CpuVisibleMapping();

		CpuVisibleMapping(CpuVisibleMapping&& other);
		CpuVisibleMapping& operator=(CpuVisibleMapping&& other);

		~CpuVisibleMapping();

		usize byte_size() const;

		void* data();
		const void* data() const;

	protected:
		void swap(CpuVisibleMapping& other);

	private:
		CpuVisibleMapping(BufferBase* base);

		void* _mapping;
		NotOwned<BufferBase*> _buffer;
};

}

#endif // YAVE_BUFFER_CPUVISIBLEMAPPING_H
