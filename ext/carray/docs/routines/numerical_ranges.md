# Numerical Range Methods

---

## arange

```php
public static arange(int $start = 0, int $stop, int $step = 1) : CArray
```
> Return evenly spaced values within a given interval.

##### Parameters

`int` **$start** (Optional) Start of interval.

`int` **$stop** End of interval.

`int` **$step** (Optional) Spacing between values.

##### Returns

`CArray` CArray of evenly spaced values.

---

## linspace

```php
public static linspace($start, $stop, $num = 50, $endpoint = True) : CArray
```
> Return evenly spaced numbers over a specified interval.

> Returns `$num` evenly spaced samples, calculated over the interval `[$start, $stop]`.

##### Parameters

`int` **$start** The starting value of the sequence.

`int` **$stop** The end value of the sequence.

`int` **$num** (Optional) Number of samples to generate.

`bool` **$endpoint** (Optional) If True, stop is the last sample. Otherwise, it is not included.

##### Returns

`CArray` There are `$num` equally spaced samples in the closed interval `[$start, $stop]`.

---

## logspace

```php
public static logspace($start, $stop, $num=50, $endpoint=True, $base=10.0) : CArray
```
> Return numbers spaced evenly on a log scale.

##### Parameters

`int` **$start** The starting value of the sequence.

`int` **$stop** The end value of the sequence.

`int` **$num** (Optional) Number of samples to generate.

`bool` **$endpoint** (Optional) If True, stop is the last sample. Otherwise, it is not included.

`int|double` **$base** (Optional) The base of the log space.

##### Returns

`CArray` `$num` samples, equally spaced on a log scale.

