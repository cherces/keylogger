Руководство по программе для перехвата системынх сообщений.
1. Для работы программы обазтельно иметь диск D, т.к для работы 
   на диске C нужны повышенные права.

Каждые 6 символов ввода меняется раскладка клавиатуры.

2. После запуска программы на диске D создаются файлы:
	D://system//
		   keylogs.txt (для запси перехваченных сообщений)
		   mydll_cp.dll (содержит перехватчик и обработчик сообщений)
3. Программа записывается в автозапуск в реестре. Данная функция отключена,
   чтобы включить нужно открыть проект и расскоментировать строчку 102 
4. После запуска exe программа работает в фоновом режиме. Чтобы отключить
   его надо зайти в диспетчер задач найти там myKeyLogger и снять задачу.
5. Для удаления созданных программой файлов может потребоваться
   закрыть все открытые программы или если это не поможет перезагрузка. 