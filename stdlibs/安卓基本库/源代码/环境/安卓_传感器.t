包名 结绳.安卓

/*
方向传感器类
*/
@导入Java("android.hardware.*")
@导入Java("android.view.WindowManager")
@后缀代码("implements SensorEventListener")
类 方向传感器 : 窗口组件
	@code
	@Override
	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == 3 && this.enabled) {
			this.yaw = event.values[0];
			this.pitch = event.values[1];
			this.roll = event.values[2];
			#方向改变(this.yaw, this.pitch, this.roll);
		}
	}
	
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}
	private boolean enabled;
	private float pitch;
	private float roll;
	private SensorManager sensors;
	private float yaw;
	private #ncls<安卓窗口> activity;
	public #cls<方向传感器>(#ncls<安卓窗口> activity) {
		super(activity);
		this.activity = activity;
		sensors = ((SensorManager) activity.getSystemService("sensor"));
		sensors.registerListener(this, this.sensors.getDefaultSensor(3), 1);
	}
	@end

	//判断手机方向传感器是否有效
	属性读 有效(): 逻辑型
		@code
		java.util.List<Sensor> sensorList = this.sensors.getSensorList(3);
		return sensorList != null && !sensorList.isEmpty();
		@end
	结束 属性

	//获取方向传感器是否可用
	属性读 可用(): 逻辑型
		@code
		return this.enabled;
		@end
	结束 属性

	//设置方向传感器是否可用
	属性写 可用(是否可用: 逻辑型)
		code this.enabled = #是否可用;
	结束 属性

	//获取方向传感器的俯仰角
	属性读 俯仰角(): 小数
		code return pitch;
	结束 属性

	//获取方向传感器的横滚角
	属性读 横滚角(): 小数
		code return roll;
	结束 属性

	//获取方向传感器的偏航角
	属性读 偏航角(): 小数
		code return yaw;
	结束 属性

	//获取倾斜角
	属性读 倾斜角(): 小数
		code return Math.toDegrees(Math.atan2(pitch,roll));
	结束 属性

	//获取屏幕旋转的角度，只能返回0,90,180,270
	方法 取屏幕旋转角度(): 整数
		@code
		  switch (((WindowManager) activity.getSystemService("window")).getDefaultDisplay().getRotation()) {
		  	 case 0:
					 return 0;
				case 1:
					 return 90;
				case 2:
					 return 180;
				case 3:
					 return 270;
				default:
					 return 0;
		  }
		@end
	结束 方法

	//方向改变时触发该事件
	定义事件 方向改变(偏航角: 小数,俯仰角: 小数,横滚角: 小数)

结束 类


/*
加速度传感器类
*/
@导入Java("android.hardware.*")
@导入Java("android.view.WindowManager")
@导入Java("java.util.*")
@后缀代码("implements SensorEventListener")
类 加速度传感器 : 窗口组件
	@code
	 private static final int SENSOR_CACHE_SIZE = 10;
	 private static final double SHAKE_THRESHOLD = 8.0d;
	 private final Queue<Float> X_CACHE = new LinkedList<>();
	 private final Queue<Float> Y_CACHE = new LinkedList<>();
	 private final Queue<Float> Z_CACHE = new LinkedList<>();
	 private boolean enabled;
	 private final Sensor sensor;
	 private float xAccel;
	 private float yAccel;
	 private float zAccel;
	 
	 private void addToSensorCache(Queue<Float> cache, float value) {
		  if (cache.size() >= 10) {
				cache.remove();
		  }
		  cache.add(Float.valueOf(value));
	 }

	 private boolean isShaking(Queue<Float> cache, float currentValue) {
		  float average = 0.0f;
		  for (Float floatValue : cache) {
				average += floatValue.floatValue();
		  }
		  return ((double) Math.abs((average / ((float) cache.size())) - currentValue)) > 8.0d;
	 }
 
	 @Override
	 public void onSensorChanged(SensorEvent event) {
		  if (event.sensor.getType() == 1 && this.enabled) {
				this.xAccel = event.values[0];
				this.yAccel = event.values[1];
				this.zAccel = event.values[2];
				addToSensorCache(this.X_CACHE, xAccel);
				addToSensorCache(this.Y_CACHE, yAccel);
				addToSensorCache(this.Z_CACHE, zAccel);
				if (isShaking(this.X_CACHE, xAccel) || isShaking(this.Y_CACHE, yAccel) || isShaking(this.Z_CACHE, zAccel)) {
					 #摇晃手机();
				}
				#加速度改变(xAccel, yAccel, zAccel);
		  }
	 }

	@Override
	public void onAccuracyChanged(Sensor s, int accuracy) {
	}

	public #cls<加速度传感器>(#ncls<安卓窗口> activity) {
		super(activity);
		  SensorManager sensors = (SensorManager) activity.getSystemService("sensor");
		  this.sensor = sensors.getDefaultSensor(1);
		  if (this.sensor != null) {
				sensors.registerListener(this, this.sensor, 1);
		  }
	}
	@end

	//判断加速度传感器是否有效
	属性读 有效(): 逻辑型
		@code
		return this.sensor != null;
		@end
	结束 属性

	//判断加速度传感器是否可用
	属性读 可用(): 逻辑型
		@code
		return this.enabled;
		@end
	结束 属性

	//设置加速度传感器是否可用
	属性写 可用(是否可用: 逻辑型)
		@code
		this.enabled = #是否可用;
		@end
	结束 属性

	//获取X方向的加速度
	属性读 X加速度(): 小数
		code return xAccel;
	结束 属性

	//获取Y方向的加速度
	属性读 Y加速度(): 小数
		code return yAccel;
	结束 属性

	//获取Z方向的加速度
	属性读 Z加速度(): 小数
		code return zAccel;
	结束 属性

	//加速度改变时触发该事件，并返回三个方向的加速度
	定义事件 加速度改变(X加速度: 小数,Y加速度: 小数,Z加速度: 小数)

	//用户摇晃手机时触发该事件
	定义事件 摇晃手机()

结束 类

/*
光线传感器类
*/
@导入Java("android.hardware.*")
@导入Java("android.view.WindowManager")
@导入Java("java.util.*")
@后缀代码("implements SensorEventListener")
类 光线传感器 : 窗口组件

	@code
	private SensorManager sensors;
	@Override
	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == 5) {
			#光线改变(event.values[0]);
		}
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}

	public #cls<光线传感器>(#ncls<安卓窗口> activity) {
		super(activity);
		sensors = (SensorManager) activity.getSystemService("sensor");
		sensors.registerListener(this, sensors.getDefaultSensor(5), 1);
	}
	@end

	//判断方向传感器是否有效
	属性读 有效(): 逻辑型
		@code
		List<Sensor> sensorList = this.sensors.getSensorList(5);
		return sensorList != null && !sensorList.isEmpty();
		@end
	结束 属性

	//当手机周围光线发生变化时触发此事件，返回光线强度，单位为勒克斯
	定义事件 光线改变(光线强度: 小数)

结束 类

/*
距离传感器类
*/
@导入Java("android.hardware.*")
@导入Java("android.view.WindowManager")
@导入Java("java.util.*")
类 距离传感器 : 窗口组件

	@code
    private #ncls<安卓环境> mContext;
	private float lastValue;
    private int mRate = 3;
    private Sensor mSensor;
    private SensorEventListener mSensorListener;
    private SensorManager mSensorManager;
    private int min = 1;
    private float value = -999.0f;
    private boolean enable;

    public #cls<距离传感器>(#ncls<安卓环境> context) {
		super(context);
        this.mContext = context;
		mSensorManager = ((SensorManager) mContext.getSystemService("sensor"));
		mSensor = mSensorManager.getDefaultSensor(8);
        if (mSensor != null) {
            enable = true;
        } else {
            enable = false;
        }
        mSensorListener = new SensorEventListener() {
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }

            public void onSensorChanged(SensorEvent event) {
                if (event.sensor.getType() == 8) {
                    value = event.values[0];
                    if (value== 0 || value < min) {
                        #物体靠近();
                    }
                    if (value == 1 || value > min) {
                        #物体离开();
                    }
                    if (lastValue != value) {
                        lastValue = value;
                        #距离改变(value);
                    }
                }
            }
        };
        if (mSensor != null) {
            mSensorManager.registerListener(this.mSensorListener, this.mSensor, this.mRate);
        }
	}
    @end

	//判断距离传感器是否可用
	属性读 可用(): 逻辑型
		@code
        return this.enable;
      @end
	结束 属性

	//设置距离传感器是否可用
	属性写 可用(是否可用: 逻辑型)
		@code
        if (this.mSensor != null) {
            if (this.enable && !#是否可用) {
                this.mSensorManager.unregisterListener(this.mSensorListener);
                this.enable = false;
            }
            if (!this.enable && #是否可用) {
                this.mSensorManager.registerListener(this.mSensorListener, this.mSensor, this.mRate);
                this.enable = true;
            }
        }
      @end
	结束 属性

	//获取物体靠近的距离
	属性读 距离(): 小数
		@code
        return this.lastValue;
      @end
	结束 属性

	//获取物体靠近的最小距离
	属性读 最小距离(): 整数
		@code
        return this.min;
      @end
	结束 属性

	//设置物体靠近的最小距离
	属性写 最小距离(最小距离: 整数)
		@code
        this.min = #最小距离;
      @end
	结束 属性

	//获取距离传感器的检测间隔时间
	属性读 检测间隔(): 整数
		@code
        return this.mRate;
      @end
	结束 属性

	//设置距离传感器的检测间隔时间
	属性写 检测间隔(间隔时间: 整数)
		@code
        this.mSensorManager.unregisterListener(this.mSensorListener);
        this.mRate = #间隔时间;
        this.mSensorManager.registerListener(this.mSensorListener, this.mSensor, this.mRate);
      @end
	结束 属性

	//当物体与手机的距离发生改变时触发此事件，返回两者的距离。
	定义事件 距离改变(距离: 小数)

	//放物体靠近手机时触发该事件
	定义事件 物体靠近()

	//当物体离开手机时触发该事件
	定义事件 物体离开()
结束 类

/*
位置传感器类
*/
@附加权限(安卓权限.网络权限)
@附加权限(安卓权限.定位权限)
@附加权限("android.permission.ACCESS_COARSE_LOCATION")
@导入Java("android.hardware.*")
@导入Java("android.location.*")
@导入Java("android.os.*")
@导入Java("android.content.*")
@导入Java("android.view.WindowManager")
@导入Java("java.util.*")
类 位置传感器 : 窗口组件

	@code
    private Criteria criteria;
    private Location location;
    private LocationManager locationManager;
    private Context mContext;
    private String provider;
	private final LocationListener locationListener = new LocationListener() {
		@Override
        public void onLocationChanged(Location location) {
            if (location != null) {
				#位置改变(location.getLatitude(), location.getLongitude(), location.getAltitude(), (double) location.getSpeed(), (double) location.getBearing());
            }
        }

		@Override
        public void onProviderDisabled(String provider) {
            #设备关闭();
        }

		@Override
        public void onProviderEnabled(String provider) {
            #设备开启();
        }

		@Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            switch (status) {
                case 0:
                    #状态改变(2);
                    break;
                case 1:
                    #状态改变(3);
                    break;
                case 2:
                    #状态改变(1);
                    break;
            }
        }
    };
   
    public #cls<位置传感器>(#ncls<安卓环境> context) {
        super(context);
        this.mContext = context;
		locationManager = ((LocationManager) mContext.getSystemService("location"));
	}
	@end

	//判断GPS硬件是否有效
	属性读 有效(): 逻辑型
		@code
        if (this.locationManager == null) {
            return false;
        }
        List<String> providers = locationManager.getAllProviders();
        if (providers != null) {
            return providers.contains("gps");
        }
        return false;
      @end
	结束 属性

	//判断GPS是否已开启
	属性读 是否已开启(): 逻辑型
		@code
        return locationManager.isProviderEnabled("gps");
      @end
	结束 属性

	//获取当前纬度
	属性读 纬度(): 小数
		@code
        if (this.location != null) {
            return this.location.getLatitude();
        }
        return 0.0d;
      @end
	结束 属性

	//获取当前经度
	属性读 经度(): 小数
		@code
        if (this.location != null) {
            return this.location.getLongitude();
        }
        return 0.0d;
      @end
	结束 属性

	//获取当前海拔，单位为m
	属性读 海拔(): 小数
		@code
        if (this.location != null) {
            return this.location.getAltitude();
        }
        return 0.0d;
      @end
	结束 属性

	//获取当前速度，单位为km/h
	属性读 速度(): 小数
		@code
        if (this.location != null) {
            return (double) this.location.getSpeed();
        }
        return 0.0d;
      @end
	结束 属性

	//获取当前方向角，为当前方向与正北方向的顺时针夹角度数
	属性读 方向(): 小数
		@code
        if (this.location != null) {
            return (double) this.location.getBearing();
        }
        return 0.0d;
      @end
	结束 属性

	//获取当前定位精度，单位为m
	属性读 精度(): 小数
		@code
        if (this.location == null || !this.location.hasAccuracy()) {
            return 0.0d;
        }
        return (double) this.location.getAccuracy();
      @end
	结束 属性

	//获取当前卫星时间
	属性读 时间(): 文本
		@code
        if (this.location != null) {
            return new java.text.SimpleDateFormat("yyyy-MM-dd HH-mm-ss").format(new Date(this.location.getTime()));
        }
        return "";
      @end
	结束 属性

	//打开GPS设置界面，让用户选择是否开启GPS
	方法 打开设置界面()
		@code
        Intent intent = new Intent();
        intent.setAction("android.settings.LOCATION_SOURCE_SETTINGS");
        intent.setFlags(268435456);
        try {
            mContext.startActivity(intent);
        } catch (ActivityNotFoundException e) {
			e.printStackTrace();
            intent.setAction("android.settings.SETTINGS");
            try {
                mContext.startActivity(intent);
            } catch (Exception e2) {
				e2.printStackTrace();
            }
        }
      @end
	结束 方法

	//开始定位并监测当前位置，当获取到位置或者位置发生改变时触发位置改变事件
	方法 开始监测()
		@code
        if (this.locationManager.isProviderEnabled("gps")) {
            if (this.criteria == null) {
                this.criteria = new Criteria();
                this.criteria.setAccuracy(1);
                this.criteria.setAltitudeRequired(true);
                this.criteria.setBearingRequired(true);
                this.criteria.setCostAllowed(true);
                this.criteria.setPowerRequirement(1);
            }
            this.provider = this.locationManager.getBestProvider(this.criteria, true);
            this.locationManager.requestLocationUpdates("gps", 1000, 0.0f, this.locationListener);
            this.location = this.locationManager.getLastKnownLocation("gps");
        }
      @end
	结束 方法

	//停止定位和监测
	方法 停止监测()
		@code
        this.locationManager.removeUpdates(this.locationListener);
      @end
	结束 方法

	/*位置发生变化时触发此事件，并得到纬度、经度等相关定位信息
	海拔的单位为米，
	速度的单位为km/h，
	方向为当前方向与正北方向的顺时针夹角的度数。
	*/
	定义事件 位置改变(纬度: 小数,经度: 小数,海拔: 小数,速度: 小数,方向: 小数)

	/*
    当GPS状态发生改变时触发此事件，返回当前GPS服务状态
    状态：1、GPS服务可用 2、不在服务区 3、服务暂停。
    */
	定义事件 状态改变(状态: 整数)

	//GPS开启时触发该事件
	定义事件 设备开启()

	//GPS关闭时触发该事件
	定义事件 设备关闭()

结束 类