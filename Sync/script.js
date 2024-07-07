document.addEventListener('scroll', function () {
    const centerContainer = document.querySelector('.center-container');
    const scrollY = window.scrollY;

    if (scrollY > 300) { // Adjust this value to control when the fade-in starts
        centerContainer.style.opacity = 1;
    } else {
        centerContainer.style.opacity = 0;
    }
});
