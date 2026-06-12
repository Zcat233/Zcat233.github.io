document.addEventListener('DOMContentLoaded', () => {
    const toggleBtn = document.getElementById('theme-toggle');
    const toggleIcon = toggleBtn ? toggleBtn.querySelector('i') : null;
    const currentTheme = localStorage.getItem('theme');

    const updateThemeToggleIcon = () => {
        if (!toggleBtn || !toggleIcon) return;

        const isDarkMode = document.body.classList.contains('dark-mode');
        toggleIcon.classList.toggle('fa-sun', !isDarkMode);
        toggleIcon.classList.toggle('fa-moon', isDarkMode);
        toggleBtn.setAttribute(
            'aria-label',
            isDarkMode ? 'Switch to light mode' : 'Switch to dark mode'
        );
    };

    if (currentTheme === 'dark') {
        document.body.classList.add('dark-mode');
    }

    updateThemeToggleIcon();

    if (toggleBtn) {
        toggleBtn.addEventListener('click', () => {
            document.body.classList.toggle('dark-mode');
            const theme = document.body.classList.contains('dark-mode') ? 'dark' : 'light';
            localStorage.setItem('theme', theme);
            updateThemeToggleIcon();
        });
    }
});
