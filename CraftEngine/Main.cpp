#include <iostream>

#include <Engine/Engine.h>
//#include "Engine/Engine.h"
//"" 로 설정하는 include는 현재 경로까지 추가로 검색해서 바로 사용 가능함, <>는 현재 경로는 찾지 않아서 문제가 생긴 것.
// 프로젝트 -> 속성 -> C++ -> 일반에서 프로젝트 경로 검색 설정하면 됨

int main()
{
	// 엔진 객체 생성 및 실행
	Craft::Engine engine;
	engine.Run();
}