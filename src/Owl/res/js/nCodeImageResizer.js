// Adapted from "nCode Image Resizer for vBulletin 3.6.0 / http://www.ncode.nl/vbulletinplugins/"

NcodeImageResizer.MAXWIDTH = 481.5;
NcodeImageResizer.MAXHEIGHT = 479.7;

var vbphrase= new Array();
vbphrase['ncode_imageresizer_warning_small'] = 'Click this bar to view the full image.';
vbphrase['ncode_imageresizer_warning_filesize'] = 'This image has been resized. Click this bar to view the full image.';
vbphrase['ncode_imageresizer_warning_no_filesize'] = 'This image has been resized. Click this bar to view the full image.';
vbphrase['ncode_imageresizer_warning_fullsize'] = 'Click this bar to view the small image.';

NcodeImageResizer.IMAGE_ID_BASE = 'ncode_imageresizer_container_';
NcodeImageResizer.WARNING_ID_BASE = 'ncode_imageresizer_warning_';
NcodeImageResizer.scheduledResizes = [];

function NcodeImageResizer(id, img) 
{
	this.id = id;
	this.img = img;
	this.originalWidth = 0;
	this.originalHeight = 0;
	this.overlay = null;

	this.originalWidth = img.originalWidth;
	this.originalHeight = img.originalHeight;
	
	img.id = NcodeImageResizer.IMAGE_ID_BASE+id;
}

NcodeImageResizer.executeOnload = function() {
	var rss = NcodeImageResizer.scheduledResizes;
	for(var i = 0; i  < rss.length; i++) {
		NcodeImageResizer.createOn(rss[i], true);
	}
}

NcodeImageResizer.schedule = function(img) {
	if(NcodeImageResizer.scheduledResizes.length == 0) {
		if(window.addEventListener) {
			window.addEventListener('load', NcodeImageResizer.executeOnload, false);
		} else if(window.attachEvent) {
			window.attachEvent('onload', NcodeImageResizer.executeOnload);
		}
	}
	NcodeImageResizer.scheduledResizes.push(img);
}

NcodeImageResizer.getNextId = function() {
	var id = 1;
	while(document.getElementById(NcodeImageResizer.IMAGE_ID_BASE+id) != null) {
		id++;
	}
	return id;
}

NcodeImageResizer.createOnId = function(id) {
	return NcodeImageResizer.createOn(document.getElementById(id));
}

NcodeImageResizer.createOn = function(img, isSchedule) 
{
	if(typeof isSchedule == 'undefined') 
	{
		isSchedule = false;
	}
	
	if(!img || !img.tagName || img.tagName.toLowerCase() != 'img') 
	{
		alert(img+' is not an image ('+img.tagName.toLowerCase()+')');
	}
	
	if(img.width == 0 || img.height == 0) 
	{
		if(!isSchedule)
		{
			NcodeImageResizer.schedule(img);
		}

		return;
	}

	if (img.height < 100 && img.width < 100)
	{
		return;
	}

	if(!img.originalWidth)
	{ 
		img.originalWidth = img.width;
	}

	if(!img.originalHeight) 
	{
		img.originalHeight = img.height;
	}
	
	var overlay = document.getElementById(img.id+"_overlay");
	if (overlay)
	{
		overlay.style["left"] = img.width - (32 + 2);
		overlay.style["bottom"] = 2;
	}

	var newid = NcodeImageResizer.getNextId();
	var resizer;
	resizer = new NcodeImageResizer(newid, img);
	resizer.createOverlay();
	resizer.scale();
}

NcodeImageResizer.prototype.createOverlay = function()
{
	var divParent = document.createElement('div');
	divParent.style["position"] = "relative";

	var overlayImg = document.createElement('img');
	overlayImg.id = this.img.id + "_overlay";
	overlayImg.className = "image_overlay";
	overlayImg.src = "qrc:/images/view-image.png";

	this.img.parentNode.insertBefore(divParent, this.img);
	this.img.parentNode.removeChild(this.img);

	divParent.appendChild(this.img);
	divParent.appendChild(overlayImg);

	overlayImg.clickData = this;
	this.overlay = overlayImg;
}

NcodeImageResizer.prototype.scale = function() 
{
	this.img.height = this.originalHeight;
	this.img.width = this.originalWidth;
	
	if(this.img.width > NcodeImageResizer.MAXWIDTH) 
	{
		this.img.height = (NcodeImageResizer.MAXWIDTH / this.img.width) * this.img.height;
		this.img.width = NcodeImageResizer.MAXWIDTH;
	}
	
	if(this.img.height > NcodeImageResizer.MAXHEIGHT) 
	{
		this.img.width = (NcodeImageResizer.MAXHEIGHT / this.img.height) * this.img.width;
		this.img.height = NcodeImageResizer.MAXHEIGHT;
	}
	
	this.overlay.style["bottom"] = 2;
	this.overlay.style["left"] = this.img.width - 34;
	this.overlay.onclick = function() { return this.clickData.overlayClicked(); }

	return false;
}

function toDataUrl(url, callback) 
{
    var xhr = new XMLHttpRequest();
	
    xhr.onload = function() 
	{
        var reader = new FileReader();
        reader.onloadend = function() 
			{
				callback(reader.result,"");
			};
        reader.readAsDataURL(xhr.response);
    };

	xhr.onreadystatechange = function()
	{
		if (xhr.readyState === 4 && xhr.status != 200)
		{   
			callback("", url);
    	} 
	}
    
	xhr.open('GET', url);
    xhr.responseType = 'blob';
    xhr.send();
}

NcodeImageResizer.prototype.overlayClicked = function() 
{
	toDataUrl(this.img.src, function(base64, url)
	{
		window.postlist.doViewImage(base64, url);
	});

	return false;
}