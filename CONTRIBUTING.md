# Contributing

Even though this project is maintained exclusively by me, André Medeiros, please, feel free to contribute in whichever way you feel like!

However, please note I will only accept PRs that satisfiy the following guidelines:

## Documentation

- Your changes should be documented as `Doxygen` comments.
- If your PR changes any public API, write usage guidelines in `README.md`.
- Before submiting your PR, generate the documentation and commit it along (`make docs`).
- Commit as regularly and meaningfully as possible (=granular and well-commented commits).

## Testing

- Write unit tests for anything you change. 
- Good tests should contemplate most common scenarios and cover all reachable code.
- Always run code coverage tests before submitting to ensure you properly tested everything (`make coverage`).

## Design guideline

- Name every public symbol as `uel_[module]_[something](_[otherthing]...)`. \
    *E.g.*: `uel_event_timer_pause` -> `event`=module, `timer`=object, `pause`=function
- Don't `malloc`. Dynamic allocation is best left to the hands of the programmer.
- Function return type, arguments and brackets all go in the same line.
- Prefer descriptive names rather than short and cryptic ones.
- Use four tabs for indentation.
- Overall, try to respect the ongoing style.

## Final notice and contact

I might also not accept your PR if I find the changes are not in line with what this framework aims to be. 
If you are unsure whether the functionality you are intending to implement or the way you plan to implement it are going to be approved, feel free to reach me by e-mail (andre@setadesenvolvimentos.com.br) so we can discuss the subject.


Thank you for your interest and time!
