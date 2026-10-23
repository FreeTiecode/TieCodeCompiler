包名 结绳.安卓

/*
TTS文字转语音工具类
提供文字转语音播放以及转换为wav音频文件功能
*/
@导入Java("android.speech.tts.TextToSpeech")
@导入Java("java.util.*")
@导入Java("android.content.*")
类 TTS : 窗口组件

	@code
	private Context context;
	private TextToSpeech mSpeech;
	
	public #cls<TTS>(Context context) {
		super(context);
		this.context = context;
		mSpeech = new TextToSpeech(context, new TTSListener());
		mSpeech.setLanguage(Locale.CHINESE);
	}
	
	private class TTSListener implements TextToSpeech.OnInitListener {
		@Override
		public void onInit(int status) {
			if (status == TextToSpeech.SUCCESS) {
				#初始化完毕(true);
			} else {
				#初始化完毕(false);
			}
		}
	}
	@end

	//设置朗读和转文件的语速
	属性写 语速(语速: 小数)
		code mSpeech.setSpeechRate((float) #语速);
	结束 属性

	//设置朗读和转文件的语速，音调越高，越偏向女声
	属性写 音调(音调: 小数)
		code mSpeech.setPitch((float) #音调);
	结束 属性

	//设置语言环境
	属性写 语言(语言: 语言环境)
		code mSpeech.setLanguage(#语言);
	结束 属性

	//获取TTS朗读状态，若正在朗读，则返回真，反之则假
	属性读 朗读状态(): 逻辑型
		code return mSpeech.isSpeaking();
	结束 属性

	//朗读一段文本
	方法 朗读文本(欲朗读文本: 文本)
		code mSpeech.speak(#欲朗读文本, TextToSpeech.QUEUE_FLUSH, null);
	结束 方法

	//将文本转为wav语音文件
	方法 文本转wav(欲转换文本: 文本,保存路径: 文本)
		@code
		HashMap<String, String> myHashRender = new HashMap<>();
		myHashRender.put(TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID, #欲转换文本);
		mSpeech.synthesizeToFile(#欲转换文本, myHashRender, #保存路径);
		@end
	结束 方法

	//关闭TTS
	方法 关闭()
		code mSpeech.shutdown();
	结束 方法

	//停止朗读
	方法 停止()
		code mSpeech.stop();
	结束 方法

	//TTS初始化完毕触发该事件，返回是否初始化成功
	定义事件 初始化完毕(结果: 逻辑型)

结束 类