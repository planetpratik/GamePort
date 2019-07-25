#pragma once

#include <windows.h>
#include <exception>
#include <string>
#include <sstream>

namespace DX
{
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr, const char* const message = "") :
			std::exception(message), mResult(hr)
		{
		}

		virtual const char* what() const override
		{
			std::stringstream what;
			what << "Failure with HRESULT of " << static_cast<unsigned int>(mResult);

			return what.str().c_str();
		}

	private:
		HRESULT mResult;
	};

	inline void ThrowIfFailed(HRESULT hr, const char* const message = "")
	{
		if (FAILED(hr))
		{
			throw com_exception(hr, message);
		}
	}
}