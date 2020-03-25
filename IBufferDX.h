#pragma once
#include <wrl/client.h>

namespace DX
{
	template <typename T>
	class IBuffer
	{
	public:
		IBuffer() = default;
		virtual ~IBuffer();

		T* GetBuffer() const noexcept { return mBuffer.Get(); }
	protected:
		Microsoft::WRL::ComPtr<T> mBuffer = nullptr;
	};

	template<typename T>
	inline IBuffer<T>::~IBuffer()
	{
		mBuffer.Reset();
	}
}
