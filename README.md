# ClipLock-DLL


## 프로젝트 소개
해당 프로젝트는 유니코드 및 PNG 형식에 대한 클립보드 보안 프로젝트입니다.

시라니오는 아래와 같습니다.
1. Build 결과 생성된 DLL(Dynamic Link Library)를 보안 대상 애플리케이션에 주입시키기
2. 이후, Win32의 SetClipboardData, GetClipboardData API를 후킹
   - SetClipboardData API로 클립보드에 유니코드 및 PNG 형식의 데이터를 넣으려고 할 때, 우회 함수에서 데이터를 암호화
   - GetClipboardData API로 클립보드에서 유니코드 및 PNG 형식의 데이터를 꺼내가려고 할 때, 우회 함수에서 암호화된 데이터를 복호화
3. 보안 대상 애플리케이션에서 넣은 유니코드 및 PNG 형식의 클립보드 데이터를 보안 대상이 아닌 애플리케이션에서 사용하려고 할 시, 암호화된 데이터가 사용됨


## 기술 스택
- MS사에서 공개한 Windows API 후킹 라이브러리 Detours를 사용해서 클립보드 API 후킹 및 제어
- OpenSSL 라이브러리의 AES-256 암호화 알고리즘을 사용해서 데이터를 암복호화
- PNG관련 zlib, libpng 라이브러리를 사용해서 Truecolor with alaph타입의 PNG 이미지를 추출하고 암복호화 진행
