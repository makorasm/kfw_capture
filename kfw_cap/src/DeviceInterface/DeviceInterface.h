#ifndef _VIDEO_INTERFACE_H_
#define _VIDEO_INTERFACE_H_

#include "semaphore.h"
#include "list.h"
#pragma pack(4)

struct s_Window
{
	int top;
	int bottom;
	int left;
	int right;
};

enum e_ExposureMode {ExposureMode__AUTO = 0, ExposureMode__MANUAL = 1};
enum e_ExposurePriority {ExposurePriority__LowNoise = 0, ExposurePriority__FrameRate = 1};
struct s_Exposure
{
	enum e_ExposureMode Mode; //dn2__ExposureMode__AUTO = 0, dn2__ExposureMode__MANUAL = 1
	enum e_ExposurePriority Priority; //dn2__ExposurePriority__LowNoise = 0, dn2__ExposurePriority__FrameRate = 1
	struct s_Window Window;
	//0..1 наверное
	float MinExposureTime;
	float MaxExposureTime;
	float MinGain;
	float MaxGain;
	float MinIris;
	float MaxIris;
	float ExposureTime;
	float Gain;
	float Iris;

};

enum e_AutoFocusMode {AutoFocusMode__AUTO = 0, AutoFocusMode__MANUAL = 1};
struct s_Focus
{
	enum e_AutoFocusMode  AutoFocusMode;	//dn2__AutoFocusMode__AUTO = 0, dn2__AutoFocusMode__MANUAL = 1
	float DefaultSpeed;
	float NearLimit;
	float FarLimit;
};

enum e_WideDynamicMode {WideDynamicMode__OFF = 0, WideDynamicMode__ON = 1};
struct s_WideDynamicRange
{
	enum e_WideDynamicMode Mode;	// required element of type dn2:WideDynamicMode
	float Level;	// optional element of type xsd:float
};

enum e_WhiteBalanceMode {WhiteBalanceMode__AUTO = 0, WhiteBalanceMode__MANUAL = 1};
struct s_WhiteBalance
{
	enum e_WhiteBalanceMode Mode;	// required element of type dn2:WhiteBalanceMode
	float CrGain;	// required element of type xsd:float
	float CbGain;	// required element of type xsd:float
};

struct s_Resolution
{
	int Width;
	int Height;
};

enum e_BacklightCompensationMode {BacklightCompensationMode__OFF = 0, BacklightCompensationMode__ON = 1};
struct s_BacklightCompensation
{
	enum e_BacklightCompensationMode Mode;	// required element of type dn2:BacklightCompensationMode
	float Level;	// required element of type xsd:float
};

enum e_IrCutFilterMode {IrCutFilterMode__ON = 0, IrCutFilterMode__OFF = 1, IrCutFilterMode__AUTO = 2};
struct s_ImagingSettings
{
	struct s_BacklightCompensation BacklightCompensation ;
	float Brightness;
	float ColorSaturation;
	float Contrast;
	float Sharpness;
	enum e_IrCutFilterMode IrCutFilter; //dn2__IrCutFilterMode__ON = 0, dn2__IrCutFilterMode__OFF = 1, dn2__IrCutFilterMode__AUTO = 2

	struct s_Exposure Exposure;
	struct s_Focus Focus;
	struct s_WideDynamicRange WideDynamicRange;
	struct s_WhiteBalance WhiteBalance;
};

/*
struct s_VideoSourceParam
{
	char VideoSourceName[255];
	float FrameRate;
	struct s_Resolution Resolution;
	struct s_ImagingSettings ImagingSettings;
};
*/
struct s_ItemListDescription_SimpleItemDescription
{
	char *Name;	// required attribute of type xsd:string
	char *Type;	// required attribute of type xsd:QName
};

struct s_ItemListDescription_ElementItemDescription
{
	char *Name;	// required attribute of type xsd:string
	char *Type;	// required attribute of type xsd:QName
};

struct s_ItemListDescription
{
	int __sizeSimpleItemDescription;	// sequence of elements <SimpleItemDescription>
	struct s_ItemListDescription_SimpleItemDescription *SimpleItemDescription;	// optional element of type dn2:ItemListDescription-SimpleItemDescription
	int __sizeElementItemDescription;	// sequence of elements <ElementItemDescription>
	struct s_ItemListDescription_ElementItemDescription *ElementItemDescription;	// optional element of type dn2:ItemListDescription-ElementItemDescription
};

struct s_ConfigDescription_Messages
{
	struct s_ItemListDescription Source;	// optional element of type dn2:ItemListDescription
	struct s_ItemListDescription Key;	// optional element of type dn2:ItemListDescription
	struct s_ItemListDescription Data;	// optional element of type dn2:ItemListDescription
	char IsProperty;	//bool
};

struct s_ConfigDescription
{
	struct s_ItemListDescription *Parameters;	// required element of type dn2:ItemListDescription
	int __sizeMessages;	// sequence of elements <Messages>
	struct s_ConfigDescription_Messages *Messages;	// optional element of type dn2:ConfigDescription-Messages
	char *Name;	// required attribute of type xsd:QName
};

struct s_SupportedAnalyticsModules
{
	int __sizeAnalyticsModuleContentSchemaLocation;	// sequence of elements <AnalyticsModuleContentSchemaLocation>
	char **AnalyticsModuleContentSchemaLocation;	// optional element of type xsd:anyURI
	int __sizeAnalyticsModuleDescription;	// sequence of elements <AnalyticsModuleDescription>
	struct s_ConfigDescription *AnalyticsModuleDescription;	// optional element of type dn2:ConfigDescription
};

struct s_VideoRateControl
{
	int FrameRateLimit;	// required element of type xsd:int
	int EncodingInterval;	// required element of type xsd:int
	int BitrateLimit;	// required element of type xsd:int
};

enum e_Mpeg4Profile {Mpeg4Profile__SP = 0, Mpeg4Profile__ASP = 1};
struct s_Mpeg4Configuration
{
	int GovLength;	// required element of type xsd:int
	enum e_Mpeg4Profile Mpeg4Profile;	// required element of type dn2:Mpeg4Profile
};

enum e_H264Profile {H264Profile__Baseline = 0, H264Profile__Main = 1, H264Profile__Extended = 2, H264Profile__High = 3};
struct s_H264Configuration
{
	int GovLength;	// required element of type xsd:int
	enum e_H264Profile H264Profile;	// required element of type dn2:H264Profile
};

enum e_VideoEncoding {VideoEncoding__JPEG = 0, VideoEncoding__MPEG4 = 1, VideoEncoding__H264 = 2};
struct s_VideoEncoderConfiguration
{
	char Name[255];	// required element of type dn2:Name
	enum e_VideoEncoding Encoding;	// required element of type dn2:VideoEncoding
	struct s_Resolution Resolution;	// required element of type dn2:VideoResolution
	float Quality;	// required element of type xsd:float
	struct s_VideoRateControl RateControl;	// optional element of type dn2:VideoRateControl
	struct s_Mpeg4Configuration MPEG4;	// optional element of type dn2:Mpeg4Configuration
	struct s_H264Configuration H264;	// optional element of type dn2:H264Configuration
};
//*********************************************************************
typedef int (pump_callback)(void*, unsigned, void*);


typedef struct _call_chain{

	struct list_head			entry;
	pump_callback* read_callback;
	void*					callback_data;

}call_chain,*pcall_chain; 


typedef struct _fpump{

	char 						sf_name[256];
	unsigned				buf_size;
	pump_callback*		pump_read_callback;//If this parameter is NULL ReadVideoData should be used
	void * 				pump_read_callback_data;
	sem_t*					stop_sem;

}fpump, *pfpump;

typedef struct _netpump{

	unsigned					server_addr;
	int								conn_type;
	unsigned short		conn_port;
	unsigned					buf_size;
	pump_callback*		pump_read_callback;

}netpump, *pnetpump;

typedef struct _omxpump{

	unsigned			buf_size;
	struct list_head			callback_chain;
	sem_t*				stop_sem;
	pid_t					omx_pid;

}omxpump, *pomxpump;

typedef struct _devpump{

}devpump, *pdevpump;

typedef union _pump_params{
	
	fpump 	file_pump_params;
	netpump net_pump_params;
	omxpump omx_pump_params;
	devpump dev_pump_params;

}pump_params, *ppump_params;

typedef enum _pump_type{
	eFILE_PUMP, 
	eNET_PUMP, 
	eOMX_PUMP, 
	eDEV_PUMP, 
}pump_type;

typedef struct s_VideoSourceParam{
	
	pump_type		p_type;
	pump_params p_params;

}VideoSourceParam, *PVideoSourceParam;

//**********************************************************************
struct  s_ProcessParam
{
	int VideoFileBufferSizeMb;
	struct s_VideoSourceParam VideoParam;
	struct s_VideoEncoderConfiguration VideoEncoderConfiguration;
	struct s_SupportedAnalyticsModules SupportedAnalyticsModules;
};


#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
// функции управления
int StartProcess();
void StopProcess();
// at the begining calling GetProcessParam and filling struct with process param, later we only change param and apply them
int SetProcessParam(struct s_ProcessParam *ProcessParam);
void GetProcessParam(struct s_ProcessParam * ProcessParam);

int GetCurrentScreenshotJPEG(char * Data, int MaxDataLen); // returning current DataLen, -1 if error
int ReadVideoData(char * Data, int DataLen); // -1,... if error
int ReadMetaData(char * Data, int DataLen); // -1,... if error
int ReadAudioData(char * Data, int DataLen); // -1,... if error

int AttachVideoCallback(pcall_chain entry);
int DetachVideoCallback(pcall_chain entry);
#ifdef __cplusplus
}
#endif
#endif //_VIDEO_INTERFACE_H_

