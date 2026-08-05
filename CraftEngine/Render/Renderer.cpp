#include "Renderer.h"
#include "ScreenBuffer.h"
#include <iostream>
#include <cassert>
#include <Windows.h>
#include "ScreenBuffer.h"


namespace Craft
{
	// ------------- Frame --------------- //
	Renderer::Frame::Frame(int bufferCount)
	{
		// 2차원 배열 생성.
		charInfoArray = std::make_unique<CHAR_INFO[]>(bufferCount);
		sortingOrderArray = std::make_unique<int[]>(bufferCount);
	}

	Renderer::Frame::~Frame()
	{
	}

	// 프레임 초기화 함수.
	void Renderer::Frame::Clear(const Vector2& screenSize)
	{
		// 이중 루프를 순회하면서 값 초기화.
		const int width = screenSize.x;
		const int height = screenSize.y;

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				// 1차원 배열을 2차원 배열로 사용할 때
				// 필요한 인덱스 좌표 변환.
				const int index = (y * width) + x;

				// 글자 항목 초기화.
				CHAR_INFO& info = charInfoArray[index];
				// 빈문자 설정 - 기존의 설정된 값 지우기.
				info.Char.AsciiChar = ' ';
				// 색상 표기 안함.
				info.Attributes = 0;

				// 그리기 순서 배열 항목 초기화.
				sortingOrderArray[index] = -1;
			}
		}
	}
	// ------------- Frame --------------- //

	// static 변수를 초기화
	Renderer* Renderer::instance = nullptr;

	Renderer::Renderer(const Vector2 screenSize)
		: screenSize(screenSize)
	{
		assert(!instance && "instance should be null");
		instance = this;

		// 콘솔 커서를 안보이게 설정, 안감추면 화면이 더 깜빡임
		// 표준 콘솔용, 업그레이드 했으니 필요없음
		//CONSOLE_CURSOR_INFO info;
		//GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		//
		//info.bVisible = FALSE;
		//SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		// 프레임 객체 생성.
		const int bufferCount = screenSize.x * screenSize.y;
		frame = std::make_unique<Frame>(bufferCount);

		// 생성 후 프레임 지우기
		frame->Clear(screenSize);

		// 이중 버퍼 구현을 위한 콘솔 버퍼 생성 및 초기화
		screenBufferAttay[0] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferAttay[0]->Clear();
		
		screenBufferAttay[1] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferAttay[1]->Clear();

		// 화면에 0번 콘솔 버퍼 활성화
		// 핸들값을 반환함.
		SetConsoleActiveScreenBuffer(screenBufferAttay[0]->GetBuffer());

	}

	Renderer::~Renderer()
	{
		instance = nullptr;

		// 표준 콘솔용, 업그레이드 했으니 필요없음
		// 콘솔 커서 다시 보이게 설정(복구)
		//CONSOLE_CURSOR_INFO info;
		//GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		//
		//// 보이기 옵션을 true로 설정
		//info.bVisible = TRUE;
		//SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		// 콘솔 창을 원래대로 복구
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));

	}

	void Renderer::Submit(const std::string& image, 
		const Vector2& position, 
		Color color, 
		int sortingOrder)
	{
		// 렌더 명령 생성 및 값 설정
		RenderCommand command;
		command.image = image;
		command.position = position;
		command.color = color;
		command.sortingOrder = sortingOrder;

		// 렌더 큐에 명령 추가
		renderQueue.emplace_back(command);
	}

	void Renderer::Draw()
	{
		// 화면(이미지/프레임)을 지우기
		Clear();

		// 프레임그리기
		DrawRenderQueue();

		// 화면(이미지/프레임) 표시
		Present();

	}

	Renderer& Renderer::Get()
	{
		assert(instance && "instance should not be null");
		return *instance;
	}

	void Renderer::Clear()
	{
		// @Temp: 시스템 clear 함수 사용
		//system("cls");
		
		// 프레임 값을 초기화
		frame->Clear(screenSize);

		// 콘솔 버퍼 초기화
		GetCurrentBuffer()->Clear();
	}

	void Renderer::DrawRenderQueue()
	{
		// 렌더 큐  순회하면서 그리기 명령 실행
		for (const RenderCommand& command : renderQueue)
		{

			// 그릴 문자값이 없으면 건너뛰기 ( 예외처리)
			if (command.image.empty())
			{
				continue;
			}

			// y 위치가 화면을 벗어났으면 건너뛰기 
						if (command.position.y < 0 || command.position.y >= screenSize.y)
			{
				continue;
			}

			// y 가 1로 가정했기 때문에 x만 함 y가 커지면 아래처럼 y도 계산
			// 그리려는 문자열의 길이 값
			const int length = static_cast<int>(command.image.length());

			// 글자의 시작 위치
			const int startX = command.position.x;
			
			// 글자의 끝 위치
			const int endX = startX + length - 1;

			// x 위치가 화면을 벗어났는지 확인.
			if (endX < 0 || startX >= screenSize.x)
			{
				continue;
			}

			// 실제 그릴 글자의 위치 구하기.
			// 삼항 연산자 (startX < 0 ? 0 : startX)
			const int visibleStart = startX < 0 ? 0 : startX;
			const int visibleEnd = endX >= screenSize.x ? screenSize.x - 1 : endX;

			// 문자열을 루프 순회 하면서 글자를 2차원 배열에 하나씩 기록
			for (int x = visibleStart; x <= visibleEnd; ++x)
			{
				// 문자열에서 글자값을 가져올 때 사용할 인덱스
				const int sourceIndex = x - startX;

				//글자 2차원 배열의 인ㄴ덱스
				// (y * width) + x
				const int index = (command.position.y * screenSize.x) + x;

				// 정렬 순서를 비교해서 그릴지 말지를 판정
				// 이미 그려진 값이 우선순위가 높으면 건너뛰기
				//같거나 새로 그리려는 값이 우선순위가 높으면 덮어쓰기
				if (frame->sortingOrderArray[index] > command.sortingOrder)
				{
					continue;
				}

				// 2차원 배열에 글자, 속성을 설정
				frame->charInfoArray[index].Char.AsciiChar
					= command.image[sourceIndex];
				// 글자 색상 값 설정
				frame->charInfoArray[index].Attributes
					= static_cast<DWORD>(command.color);
				// 그리기 우선순위값
				frame->sortingOrderArray[index] = command.sortingOrder;

			}

			//// 원도우 콘솔 핸들
			//HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
			//
			//
			//// 그릴 위치로 콘솔 좌표 이동처리
			//SetConsoleCursorPosition(handle, command.position);
			//
			//// 글자 색상을 설정
			//SetConsoleTextAttribute(handle, static_cast<WORD>(command.color));
			//
			//// @Temp: 그리기
			//std::cout << command.image;
			//
			//// 콘솔 색상을 복원
			//SetConsoleTextAttribute(handle, static_cast<WORD>(Color::White));

			// 앞에서 설정한 2차원 배열을 콘솔에 그리기
			GetCurrentBuffer()->Draw(frame->charInfoArray.get());

		}
		// 렌더 큐 배열 비우기
		renderQueue.clear();

		// 콘솔 색상 초기화
		SetConsoleTextAttribute(
			GetCurrentBuffer()->GetBuffer(),
			static_cast<DWORD>(Color::White)
		);

	}

	void Renderer::Present()
	{
		// 현재 순번의 콘솔 버퍼를 활성화 처리
		SetConsoleActiveScreenBuffer(GetCurrentBuffer()->GetBuffer());

		// 인덱스 업데이트 (갱신)
		//currentBufferIndex = (currentBufferIndex + 1) % 2
		// 마법의 공식 -> One Minus...
		currentBufferIndex = 1 - currentBufferIndex;
	}
	const ScreenBuffer* const Renderer::GetCurrentBuffer() const
	{
		// 스마트 포인터에서 원시 포인터로 다룰 때 get (더 알아보기)
		return screenBufferAttay[currentBufferIndex].get();
	}
}