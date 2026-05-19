// Copyright 2023 CB All Rights Reserved.


#include "Tween/CBTweenBPLibrary.h"

#include "CBCommon.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Tween/CBTweenSubsystem.h"


UCBTweenBase* UCBTweenBPLibrary::FloatTo(UObject* WorldContextObject, FCBTweenFloatSetterDynamic Setter, float Start,
                                         float End, float Duration, float Delay, EEasingCategory EasingCategory, EEasingType EasingType)
{
	 UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(WorldContextObject)->To(FCBTweenFloatGetterdDelegate::CreateLambda([Start]
	{
		return Start;
	 	
	}),FCBTweenFloatSetterdDelegate::CreateLambda([Setter](float Value)
	{
		if (Setter.IsBound())
		{
			Setter.Execute(Value);
		}
	}),End,Duration);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
}

UCBTweenBase* UCBTweenBPLibrary::IntTo(UObject* WorldContextObject, FCBTweenIntSetterDynamic Setter, int32 Start,
	int32 End, float Duration, float Delay, EEasingCategory EasingCategory, EEasingType EasingType)
{
	UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(WorldContextObject)->To(FCBTweenIntGetterdDelegate::CreateLambda([Start]
	{
	 return Start;
	 	
	}),FCBTweenIntSetterdDelegate::CreateLambda([Setter](int32 Value)
	{
   if (Setter.IsBound())
   {
	   Setter.Execute(Value);
   }
	}),End,Duration);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
}

UCBTweenBase* UCBTweenBPLibrary::VetcorTo(UObject* WorldContextObject, FCBTweenVectorSetterDynamic Setter, FVector Start ,
	FVector End ,float Duration, float Delay, EEasingCategory EasingCategory, EEasingType EasingType)
{
	UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(WorldContextObject)->To(FCBTweenVectorGetterdDelegate::CreateLambda([Start]
	{
	 return Start;
	 	
	}),FCBTweenVectorSetterdDelegate::CreateLambda([Setter](FVector Value)
	{
   if (Setter.IsBound())
   {
	   Setter.Execute(Value);
   }
	}),End,Duration);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
}

UCBTweenBase* UCBTweenBPLibrary::Vetcor2DTo(UObject* WorldContextObject, FCBTweenVector2DSetterDynamic Setter,
	FVector2D Start, FVector2D End, float Duration, float Delay, EEasingCategory EasingCategory, EEasingType EasingType)
{
	UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(WorldContextObject)->To(FCBTweenVector2DGetterdDelegate::CreateLambda([Start]
	{
	 return Start;
	 	
	}),FCBTweenVector2DSetterdDelegate::CreateLambda([Setter](FVector2D Value)
	{
   if (Setter.IsBound())
   {
	   Setter.Execute(Value);
   }
	}),End,Duration);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
}

UCBTweenBase* UCBTweenBPLibrary::TransformTo(UObject* WorldContextObject, FCBTweenTransformSetterDynamic Setter,
	FTransform Start, FTransform End, float Duration, float Delay, EEasingCategory EasingCategory,
	EEasingType EasingType)
{
		UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(WorldContextObject)->To(FCBTweenTransformGetterdDelegate::CreateLambda([Start]
	{
		return Start;
	 	
	}),FCBTweenTransformSetterdDelegate::CreateLambda([Setter](FTransform Value)
	{
	if (Setter.IsBound())
	{
		Setter.Execute(Value);
	}
	}),End,Duration);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
}

UCBTweenBase* UCBTweenBPLibrary::MaterialScalarParameterTo(UObject* WorldContextObject,
                                                           UMaterialInstanceDynamic* Target, FName ParameterName, float End, float Duration, float Delay,
                                                           EEasingCategory EasingCategory, EEasingType EasingType)
{
	if (!IsValid(Target))
	{
		UE_LOG(LogCBTween, Error, TEXT("[UCBTweenBPLibrary::MaterialScalarParameterTo] target is not valid:%s"), *(Target->GetPathName()));
		return nullptr;
	}
	
	float StartValue = 0.0f;
	int32 ParameterIndex = 0;
	if (Target->GetScalarParameterValue(ParameterName, StartValue))
	{
		Target->InitializeScalarParameterAndGetIndex(ParameterName, StartValue, ParameterIndex);
	}
	else
	{
		UE_LOG(LogCBTween, Error, TEXT("[UCBTweenBPLibrary::MaterialScalarParameterTo]GetScalarParameterValue:%s error!"), *(ParameterName.ToString()));
		return nullptr;
	}
	UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(WorldContextObject)->To(FCBTweenMaterialScalarGetterDelegate::CreateWeakLambda(Target,[Target,ParameterName](float&Result)
	{
		return Target->GetScalarParameterValue(ParameterName,Result);
	}),FCBTweenMaterialScalarSetterDelegate::CreateUObject(Target,&UMaterialInstanceDynamic::SetScalarParameterByIndex),End,Duration,ParameterIndex);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
}

UCBTweenBase* UCBTweenBPLibrary::MeshMaterialScalarParameterTo(UPrimitiveComponent* Target, int32 MaterialIndex,
	FName ParameterName, float End, float Duration, float Delay, EEasingCategory EasingCategory,
	EEasingType EasingType)
{
	if (!IsValid(Target))
	{
		UE_LOG(LogCBTween, Error, TEXT("[UCBTweenBPLibrary::MaterialScalarParameterTo] target is not valid:%s"), *(Target->GetPathName()));
		return nullptr;
	}
	
	float StartValue = 0.0f;
	int32 ParameterIndex = 0;
	UMaterialInstanceDynamic* Material=Target->CreateAndSetMaterialInstanceDynamic(ParameterIndex);
	if (Material->GetScalarParameterValue(ParameterName, StartValue))
	{
		Material->InitializeScalarParameterAndGetIndex(ParameterName, StartValue, ParameterIndex);
	}
	else
	{
		UE_LOG(LogCBTween, Error, TEXT("[UCBTweenBPLibrary::MaterialScalarParameterTo]GetScalarParameterValue:%s error!"), *(ParameterName.ToString()));
		return nullptr;
	}

	UCBTweenBase* Tween=UCBTweenSubsystem::GetTweenSubsystee(Target)->To(FCBTweenMaterialScalarGetterDelegate::CreateWeakLambda(Material,[Material,ParameterName](float&Result)
	{
		return Material->GetScalarParameterValue(ParameterName,Result);
	}),FCBTweenMaterialScalarSetterDelegate::CreateUObject(Material,&UMaterialInstanceDynamic::SetScalarParameterByIndex),End,Duration,ParameterIndex);

	Tween->SetDelay(Delay);
	Tween->SetEase(EasingCategory,EasingType);

	return Tween;
	
}

