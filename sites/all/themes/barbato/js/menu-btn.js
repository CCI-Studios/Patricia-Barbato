(function($) {
	$(function(){
		$("#block-block-17 .content a").click(toggleMenu);
	});

	function toggleMenu()
	{	
		$("#block-system-main-menu .content").slideToggle();
		return false;
	}
}(jQuery));