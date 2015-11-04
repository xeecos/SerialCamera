package
{
	import flash.display.Bitmap;
	import flash.display.JPEGEncoderOptions;
	import flash.display.Loader;
	import flash.display.Shape;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.filesystem.File;
	import flash.filesystem.FileMode;
	import flash.filesystem.FileStream;
	import flash.geom.Rectangle;
	import flash.net.FileReference;
	import flash.net.SharedObject;
	import flash.utils.ByteArray;
	import flash.utils.setTimeout;
	
	import fl.controls.Button;
	import fl.controls.ComboBox;
	import fl.data.DataProvider;
	
	public class SerialCamera extends Sprite
	{
		private var _serial:AIRSerial = new AIRSerial;
		private var _btConnect:Button = new Button();
		private var _btCapture:Button = new Button();
		private var _btSave:Button = new Button();
		private var _cbPorts:ComboBox = new ComboBox();
		private var _cbResolution:ComboBox = new ComboBox();
		private var _bmp:Bitmap = new Bitmap;
		private var _loader:Loader = new Loader;
		private var _commands:Array = [];
		private var _so:SharedObject = SharedObject.getLocal("camera","/");
		private var _bg:Shape = new Shape;
		private var _precent:Number = 0;
		public function SerialCamera()
		{
			stage.align = "TL";
			stage.scaleMode = "noScale";
			this.addEventListener(Event.ADDED_TO_STAGE,onInit);
		}
		private function onInit(evt:Event):void{
			addChild(_bg);
			_btConnect.label = "Connect";
			_btSave.label = "Save";
			_btCapture.label = "Capture";
			addChild(_btConnect);
			_btConnect.addEventListener(MouseEvent.CLICK,onConnect);
			addChild(_cbPorts);
			addChild(_cbResolution);
			addChild(_btCapture);
			addChild(_btSave);
			addChild(_bmp);
			_cbResolution.setSize(180,22);
			var dp:DataProvider = new DataProvider;
			var ports:Array = _serial.list().split(",");
			for(var i:uint = 0;i<ports.length;i++){
				dp.addItem({label:ports[i],data:ports[i]});
			}
			_cbPorts.dataProvider = dp;
			_cbPorts.selectedIndex = _so.data.port;
			dp = new DataProvider;
			dp.addItem({label:"320x240",data:0});
			dp.addItem({label:"640x480",data:1});
			dp.addItem({label:"160x120",data:2});
			_cbResolution.dataProvider = dp;
			_cbResolution.addEventListener(Event.CHANGE,onResolutionChanged);
			_cbResolution.selectedIndex = _so.data.resolution;
			_btCapture.addEventListener(MouseEvent.CLICK,onCapture);
			_btSave.addEventListener(MouseEvent.CLICK,onSave);
			_serial.addEventListener(Event.CHANGE,onReceived);
			stage.addEventListener(Event.RESIZE,onResized);
			_loader.contentLoaderInfo.addEventListener(Event.COMPLETE,onLoaded);
		}
		private function onResized(evt:Event):void{
			var sw:uint = stage.stageWidth;
			var sh:uint = stage.stageHeight;
			_btCapture.x = sw-_btCapture.width-10;
			_btCapture.y = 10;
			_cbPorts.x = 10;
			_cbPorts.y = 10;
			_cbResolution.x = sw-_cbResolution.width-_btCapture.width-20;
			_cbResolution.y = 10;
			_btConnect.x = 120;
			_btConnect.y = 10;
			_btSave.x = sw - _btSave.width - 10;
			_btSave.y = sh - _btSave.height - 10;
			_bmp.x = (sw - _bmp.width)/2;
			_bmp.y = (sh - _bmp.height)/2;
			with(_bg.graphics){
				beginFill(0xeeffee,1);
				drawRect(0,0,sw*_precent,sh);
				endFill();
			}
		}
		private function onLoaded(evt:Event):void{
			_bmp.bitmapData = (_loader.content as Bitmap).bitmapData;
			stage.dispatchEvent(new Event(Event.RESIZE));
		}
		private function onConnect(evt:MouseEvent):void{
			if(_btConnect.label=="Connect"){
				if(_serial.open(_cbPorts.selectedItem.data)==0){
					trace("connected");
					_btConnect.label = "Disconnect";
					_so.data.port = _cbPorts.selectedIndex;
					_so.flush(100);
				}
			}else{
				if(_serial.isConnected){
					_serial.close();
				}
				_btConnect.label = "Connect";
			}
		}
		private function onCapture(evt:MouseEvent):void{
			_commands.push("/clear\n");
			_commands.push("/capture\n");
			_commands.push("/request/length\n");
			startCommands();
		}
		private function onResolutionChanged(evt:Event):void{
			_so.data.resolution = _cbResolution.selectedIndex;
			_so.flush(100);
			_commands.push("/resolution/"+_cbResolution.selectedItem.data+"\n");
			_commands.push("/reset\n");
			startCommands();
		}
		private var _bytes:ByteArray = new ByteArray;
		private var _length:Number = 0;
		private function onReceived(evt:Event):void{
			if(_serial.isConnected){
				var len:uint = _serial.getAvailable();
				if(len>0){
					_bytes.writeBytes(_serial.readBytes());
					var result:String = "";
					for(var i:uint=0;i<_bytes.length;i++){
						result+="0x"+_bytes[i].toString(16)+" ";						
					}
					if(_bytes.length==9){
						if(_bytes[0]==0x76&&_bytes[2]==0x34&&_bytes[4]==0x4){
							_bytes.position = 7;
							_length = _bytes.readUnsignedShort();
							trace("length:",_length);
							_commands.push("/request/buffer/0/0/"+_bytes[7]+"/"+_bytes[8]+"\n");
							startCommands();
						}
					}
					if(_length>0){
						trace("received:",_bytes.length,_length);
						_precent = _bytes.length/(_length+10);
						with(_bg.graphics){
							beginFill(0xeeffee,1);
							drawRect(0,0,stage.stageWidth*_precent,stage.stageHeight);
							endFill();
						}
						if(_bytes.length==_length+10){
							_precent = 0;
							_bg.graphics.clear();
							var bytes:ByteArray = new ByteArray();
							bytes.writeBytes(_bytes,5,_bytes.length-10);
							_loader.loadBytes(bytes);
							var file:File = File.applicationStorageDirectory.resolvePath("temp.jpeg");
							var fs:FileStream = new FileStream();
							fs.open(file,FileMode.WRITE);
							fs.writeBytes(bytes);
							fs.close();
							bytes.clear();
							_length = 0;
						}
					}
				}
			}
		}
		private function onSave(evt:MouseEvent):void{
			var file:File = new File();
			function onSaveSelected(evt:Event):void{
				if(_bmp.bitmapData){
					var fs:FileStream = new FileStream();
					fs.open(file,FileMode.WRITE);
					var jpg:JPEGEncoderOptions = new JPEGEncoderOptions();
					jpg.quality = 80;
					fs.writeBytes(_bmp.bitmapData.encode(_bmp.bitmapData.rect,jpg));
					fs.close();
				}
			}
			file.addEventListener(Event.SELECT,onSaveSelected);
			file.browseForSave("save image");
		}
		private function startCommands():void{
			if(_commands.length>0){
				if(_serial.isConnected){
					_bytes.clear();
					_serial.writeString(_commands[0]);
					_commands.shift();
					setTimeout(startCommands,500);
				}
			}
		}
	}
}