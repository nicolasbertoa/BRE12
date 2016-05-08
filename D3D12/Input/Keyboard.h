#pragma once

#include <cstdint>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <memory>

class Keyboard {
public:
	static std::unique_ptr<Keyboard> gKeyboard;

	Keyboard(IDirectInput8& directInput, const HWND windowHandle);
	Keyboard(const Keyboard&) = delete;
	const Keyboard& operator=(const Keyboard& rhs) = delete;
	~Keyboard();

	void Update();

	const std::uint8_t* const CurrentState() const { return mCurrentState; }
	const std::uint8_t* const LastState() const { return mLastState; }
	bool IsKeyUp(const std::uint8_t key) const { return (mCurrentState[key] & 0x80) == 0U; }
	bool IsKeyDown(const std::uint8_t key) const { return (mCurrentState[key] & 0x80) != 0U; }
	bool WasKeyUp(const std::uint8_t key) const { return (mLastState[key] & 0x80) == 0U; }
	bool WasKeyDown(const std::uint8_t key) const { return (mLastState[key] & 0x80) != 0U; }
	bool WasKeyPressedThisFrame(const std::uint8_t key) const { return IsKeyDown(key) && WasKeyUp(key); }
	bool WasKeyReleasedThisFrame(const std::uint8_t key) const { return IsKeyUp(key) && WasKeyDown(key); }
	bool IsKeyHeldDown(const std::uint8_t key) const { return IsKeyDown(key) && WasKeyDown(key); }

private:
	IDirectInput8& mDirectInput;
	LPDIRECTINPUTDEVICE8 mDevice{ nullptr };
	std::uint8_t mCurrentState[256U] = {};
	std::uint8_t mLastState[256U] = {};
};