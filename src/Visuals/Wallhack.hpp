//
// Created by therdel on 29.06.19.
//
#pragma once

#include <set>
#include "../Utility.hpp"

class Wallhack : public Utility::NonCopyable, public Utility::NonMovable {
public:
	Wallhack();

	~Wallhack();

private:
	// hook to library function
	// credit: https://aixxe.net/2016/12/imgui-linux-csgo
	using HRESULT = long;
	typedef enum D3DPRIMITIVETYPE {
		D3DPT_POINTLIST = 1,
		D3DPT_LINELIST = 2,
		D3DPT_LINESTRIP = 3,
		D3DPT_TRIANGLELIST = 4,
		D3DPT_TRIANGLESTRIP = 5,
		D3DPT_TRIANGLEFAN = 6,
		D3DPT_FORCE_DWORD = 0x7fffffff
	} D3DPRIMITIVETYPE, *LPD3DPRIMITIVETYPE;
	using DrawIndexedPrimitive_t =
	HRESULT (*)(uintptr_t, D3DPRIMITIVETYPE, int, unsigned, unsigned, unsigned, unsigned);

	struct IndexedPrimitive {
		IndexedPrimitive(D3DPRIMITIVETYPE d3Dprimitivetype,
		                 int BaseVertexIndex,
		                 unsigned MinVertexIndex,
		                 unsigned NumVertices,
		                 unsigned startIndex,
		                 unsigned primCount)
				: d3Dprimitivetype(d3Dprimitivetype)
				, BaseVertexIndex(BaseVertexIndex)
				, MinVertexIndex(MinVertexIndex)
				, NumVertices(NumVertices)
				, startIndex(startIndex)
				, primCount(primCount) {}

		D3DPRIMITIVETYPE d3Dprimitivetype;
		int BaseVertexIndex;
		unsigned MinVertexIndex;
		unsigned NumVertices;
		unsigned startIndex;
		unsigned primCount;

		bool operator<(IndexedPrimitive const &rhs) const {
			if (d3Dprimitivetype != rhs.d3Dprimitivetype) {
				return d3Dprimitivetype < rhs.d3Dprimitivetype;
			} else if (BaseVertexIndex != rhs.BaseVertexIndex) {
				return BaseVertexIndex < rhs.BaseVertexIndex;
			} else if (MinVertexIndex != rhs.MinVertexIndex) {
				return MinVertexIndex < rhs.MinVertexIndex;
			} else if (NumVertices != rhs.NumVertices) {
				return NumVertices < rhs.NumVertices;
			} else if (startIndex < rhs.startIndex) {
				return startIndex < rhs.startIndex;
			} else {
				return primCount < rhs.primCount;
			}
		}
	};

	std::set<IndexedPrimitive> m_indexedPrimitives;

	const uintptr_t m_shaderapidx9Base;
	const DrawIndexedPrimitive_t m_orig_DrawIndexedPrimitive;

	static DrawIndexedPrimitive_t getDrawIndexedPrimitive();

	// hooking stuff
	void installDrawIndexedPrimitiveHook();

	void removeDrawIndexedPrimitiveHook();

	static HRESULT hook_DrawIndexedPrimitive(
			uintptr_t thisptr,
			D3DPRIMITIVETYPE d3Dprimitivetype,
			int BaseVertexIndex,
			unsigned MinVertexIndex,
			unsigned NumVertices,
			unsigned startIndex,
			unsigned primCount);
};

extern Wallhack *g_Wallhack;