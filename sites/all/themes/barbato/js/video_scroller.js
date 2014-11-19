(function($) {
	var active = 0;
	var min = 0;
	var max = 0;
	var timer;
	var paused = false;

	$(function()
	{
		$("#prev").click(clickPrevious);
		$("#next").click(clickNext);
		$(".view-media-logos .views-row").click(clickThumbnail);
		$(".view-media-video-list-view").on("mouseenter", pause).on("mouseleave", unpause).on("click", stop);
		max = rows().length;

		var $first = rows().eq(0).clone();

		container().append($first);

		setTimeout(layout, 50);
		$(window).resize(layout);

		addActive();
		start();

	});

	function addActive()
	{
		$(".views-row-first").addClass("active");

	}

	function start()
	{	

		timer = setInterval(timerNext, 7000);

	}

	function container()
	{
		return $("#block-views-media-video-list-view-block .view-content");
	}
	function rows()
	{
		return container().find(".views-row");
	}

	function indicators()
	{
		return $("#block-views-media-logos-block .views-row");
	}

	function layout()
	{
		var numRows = rows().length;
		var containerWidth = numRows * rowWidth();
		var width = 1/numRows*100
		container().width(containerWidth+"%");
		rows().width(width+"%");


	}

	function moveContainer()
	{
		var left = "-" + (active*rowWidth()) + "%";
		container().stop(false, false).animate({"left":left},1500);
		setActiveIndicator(active);
	}
	function jumpToEnd()
	{
		var active = rows().length-rowsPerPage();
		var left = "-" + (active*rowWidth()) + "%";
		container().css({"left":left});
	}
	function jumpToBeginning()
	{
		var active = min;
		var left = "-" + (active*rowWidth()) + "%";
		container().css({"left":left});
	}
	function rowWidth()
	{
		return 100/rowsPerPage();
	}
	function rowsPerPage()
	{
		if (isMobile())
		{
			return 1;
		}
		else if (isTablet())
		{
			return 1;
		}
		
		return 1;
	}
	function isMobile()
	{
		return $(window).width() < 540;
	}
	function isTablet()
	{
		return $(window).width() < 1220;
	}

	function previous()
	{
		active--;
		if (active < min)
		{
			jumpToEnd();
			active = max-1;
		}
		moveContainer();
	}

	function next()
	{
		active++;
		if (active > max)
		{
			jumpToBeginning();
			active = min+1;
		}
		moveContainer();
	}

	function gotoIndex(i)
	{
		active = i;
		moveContainer();
	}

	function clickPrevious()
	{
		previous();
		stop();
		return false;
	}
	function clickThumbnail()
	{
		var i = $(this).index();
		setActiveIndicator(i);
		gotoIndex(i);
		stop();
	}
	function clickNext()
	{
		next();
		stop();
		return false;
	}

	function stop()
	{
		clearInterval(timer);
	}
	function pause()
	{
		paused = true;
	}
	function unpause()
	{
		paused = false;
	}

	function timerNext()
	{
		if (!paused)
		{
			next();
		}
		layout();
	}

	function setActiveIndicator(i)
	{
		if (i >= max)
		{
			i = 0;
		}
		indicators().removeClass("active").eq(i).addClass("active");
	}


}(jQuery));
