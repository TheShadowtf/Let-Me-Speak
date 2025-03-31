@echo off
echo Pulling latest changes from origin...
git pull origin main --allow-unrelated-histories

echo.
echo Adding all changes...
git add .

echo.
set /p msg="Enter commit message: "
git commit -m "%msg%"

echo.
echo Pushing to GitHub...
git push origin main

echo.
echo ✅ Done! Press any key to close...
pause > nul
