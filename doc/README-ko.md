<p align="center">
  <img src="woterm.png" width="64" alt="WoTerm - Another Remote Access Assistant">
  <h2 style="text-align: center;">크로스 플랫폼 / 저메모리 / 다중 프로토콜 / 다기능 / 클라우드 동기화</h2>
  <h3 style="text-align: center;">
  [<a href="../README.md">English</a>] | 
  [<a href="README-zh_CN.md">简体中文</a>]
  [<a href="README-zh_TW.md">繁體中文</a>]
  [<a href="README-de.md">Deutsch</a>]
  [<a href="README-es.md">Español</a>]
  [<a href="README-fr.md">Français</a>]
  [<a href="README-hi.md">हिंदी</a>]
  [<a href="README-id.md">Bahasa Indonesia</a>]
  [<a href="README-ja.md">日本語</a>]
  [<a href="README-ko.md">한국어</a>]
  [<a href="README-ru.md">Русский</a>]
</h3>
  <h3 style="text-align: center;">[<a href="https://woterm.com">WoTerm 공식 웹사이트</a>]</a></h3>
</p>

# 개요
주요 원격 통신 프로토콜 통합: SSH1/SSH2, FTP/FTPS, SFTP, TELNET, RLOGIN, RDP, VNC, SHELL, 시리얼 포트, TCP, UDP 등 지원 — 도구 전환 불필요.  
통합 플랫폼에서 다양한 원격 액세스 시나리오를 쉽게 처리.

## 기능 설명[<a href="https://en.woterm.com/versions/">&gt;&gt;&gt;더 많은 기능을 보려면 여기를 클릭하세요&lt;&lt;&lt;</a>]
- Windows / Linux / MacOSX / Android 지원.  
- 주요 통신 프로토콜 지원: SSH1 / SSH2 / SFTP / RLOGIN / TELNET / SERIALPORT / VNC / RDP 등.  
- 여러 내장 스킨, 확장 가능, 고급 사용자가 자유롭게 커스터마이징 가능.  
- 관리자 모드: 세션 비밀번호 확인 및 도난 방지.  
- SFTP 기반 클라우드 동기화 프로토콜, 8개 고강도 암호화 알고리즘.  
- SSH 터미널에 내장 SFTP 도우미, 독립 탭도 지원.  
- 플로팅 탭 디자인, 다중 창 협업에 편리.  
- 무제한 창 분할.  
- 세션 그룹 및 키워드 필터.  
- 수십 가지 터미널 색상 선택 가능.

- VNC 원격 데스크톱 지원, 고급 기능은 [WoVNCServer](http://wovnc.com) 필요.  
  - RFB 3.3/3.7/3.8 표준 프로토콜.  
  - 표준 인코딩: ZRLE / TRLE / Hextile / CopyRect / RRE / Raw.  
  - 확장 인코딩: H264 / JPEG / ZRLE3 / TRLE3 / ZRLE2 / TRLE2.  
  - 네트워크 연결 끊김 또는 원격 작업 종료 시 자동 화면 잠금.  
  - 원격 블랙 스크린 모드.  
  - 원격 데스크톱 사운드 재생.  
  - 이미지 품질 전환 가능: 무손실, HD, 일반, 16/15/8비트.  
  - 멀티 스크린 및 스트레치, 공유, 분할 모드 지원.

- 원격 터미널 지원.  
  - Windows/Linux/MacOSX 터미널 액세스.  
  - 프록시 점프 지원.  
  - 인증서 관리.  
  - 단축키 설정.  
  - ZModem 파일 업로드/다운로드.  
  - 내장 SFTP 협업.  
  - 다양한 터미널 색상 선택 가능.

## 모듈 설명
클라이언트 코드는 완전히 오픈소스(kxver 모듈 제외), 서드파티 코드는 GitHub, CodeProject 또는 다른 오픈 커뮤니티 출처. 일부 모듈은 [WoVNC](http://wovnc.com)에서 가져옴.

## WoVNCServer
고급 기능 사용을 위해 [WoVNCServer](http://wovnc.com) 권장.

## 추가 정보:
<a href="http://www.woterm.com">http://www.woterm.com</a>

## 바이너리 다운로드:
<a href="http://woterm.com">http://woterm.com</a>

## 성능

자체 개발 터미널 파싱 엔진, 수 초 내에 수천만 줄을 파싱 및 표시하며 매우 낮은 메모리 사용을 유지합니다. [자세한 과정 보기](Performance-ko.md)

![](timeseq1.png)  
![](urandom_test_speed.png)  
![](urandom_test_memory.png)  

수백만 줄 텍스트에서 초고속 검색 및 위치 지정  
![](search.gif)

---

## UI 미리보기

**메인 UI**  
![](main.gif)  

**SSH 키 관리**  
![](keymgr2.gif)  

**시리얼 포트**  
![](serialport.gif)  

**터널**  
![](tunnel.png)  

**다중 스킨**  
![](skins.png)  

**플레이북**  
![](playbook.gif)  

**병합 및 분할**  
![](merge.gif)  

**클라우드 동기화**  
![](sync.gif)  

**키워드 필터**  
![](filter.gif)  

**파일 전송**  
![](sftp.gif)  

**플로팅 탭**  
![](float.gif)  

**창 분할**  
![](split.gif)  

**터미널 테마**  
![](patten.gif)  

**구문 강조 표시**  
![](highlight.gif)  

**VNC 터미널**  
![](vnc.gif)


# 오픈 소스 성명
v10부터 우리는 오픈 소스 라이선스를 조정했습니다. 앞으로 부분 오픈 소스를 유지하든, 점차 완전한 오픈 소스로 나아가든, 우리는 항상 개방적이고 투명한 태도를 유지할 것입니다.  

이 저장소에는 타사 라이브러리나 **kxver 모듈**의 소스 코드/라이브러리 파일이 포함되어 있지 않습니다.  
**kxver 모듈**은 주로 버전 제어에 사용되며, 특히 데이터 암호화와 같은 프로페셔널 버전 기능과 관련이 있습니다.  
이 모듈은 또한 팀의 유일한 자금 출처이며, 제품의 건전한 유지 관리와 지속적인 업그레이드를 위한 중요한 보장입니다.  
자금 문제를 해결하기 전까지는 이 모듈을 오픈 소스로 공개할 계획이 없습니다.  

전체적으로 우리의 목표는 두 가지입니다:  
1. **적절한 상업화를 통해 합리적인 수익 창출** — 지속적인 개발과 유지 관리를 위한 장기적인 동력을 제공하기 위해;  
2. **사용자 감사가 가능하도록 코드 유지** — 모든 사람이 소프트웨어가 "악하지 않는다"는 원칙을 준수한다는 것을 확인하고 안심하고 사용할 수 있도록.  

우리는 초심을 지키는 동시에 지속 가능한 발전을 보장하여 소프트웨어가 안정적으로, 그리고 오래도록 발전하기를 바랍니다.  
