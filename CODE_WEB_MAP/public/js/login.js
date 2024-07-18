document.addEventListener("DOMContentLoaded", function () {
  const loginForm = document.getElementById("login");
  const mainContent = document.getElementById("main-content");
  const logoutButton = document.getElementById("logout");

  loginForm.addEventListener("submit", function (event) {
    event.preventDefault();

    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;

    // Example authentication check (replace with real authentication logic)
    if (username === "admin" && password === "password") {
      document.getElementById("login-form").style.display = "none";
      mainContent.style.display = "block";
    } else {
      alert("Invalid username or password");
    }
  });

  logoutButton.addEventListener("click", function (event) {
    event.preventDefault();
    document.getElementById("login-form").style.display = "block";
    mainContent.style.display = "none";
  });
});
