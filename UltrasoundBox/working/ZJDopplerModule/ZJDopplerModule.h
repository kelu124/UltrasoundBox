// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� ZJDOPPLERMODULE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// ZJDOPPLERMODULE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef ZJDOPPLERMODULE_EXPORTS
#define ZJDOPPLERMODULE_API __declspec(dllexport)
#else
#define ZJDOPPLERMODULE_API __declspec(dllimport)
#endif

// �����Ǵ� ZJDopplerModule.dll ������
//class ZJDOPPLERMODULE_API CZJDopplerModule {
//public:
//	CZJDopplerModule(void);
//	// TODO:  �ڴ�������ķ�����
//};
//
//extern ZJDOPPLERMODULE_API int nZJDopplerModule;
//
//ZJDOPPLERMODULE_API int fnZJDopplerModule(void);
